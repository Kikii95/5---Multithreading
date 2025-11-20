#include "CollisionDetector.h"
#include <stdio.h>

// Thread de monitoring de progression
DWORD WINAPI ProgressMonitorThread(LPVOID param) {
    ProgressMonitor* monitor = (ProgressMonitor*)param;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    int startY = csbi.dwCursorPosition.Y;

    while (InterlockedCompareExchange(&monitor->running, 0, 0) == 1) {
        LONG completed = InterlockedCompareExchange(&monitor->comparisonsCompleted, 0, 0);
        float progress = (float)completed / monitor->totalComparisons * 100.0f;

        DWORD elapsed = GetTickCount() - monitor->startTime;
        float elapsedSeconds = elapsed / 1000.0f;

        // Estimation temps restant
        float remainingSeconds = 0;
        if (completed > 0) {
            float timePerComparison = elapsedSeconds / completed;
            remainingSeconds = timePerComparison * (monitor->totalComparisons - completed);
        }

        // Barre de progression
        COORD coord;
        coord.X = 0;
        coord.Y = startY;
        SetConsoleCursorPosition(hConsole, coord);

        printf("  [");
        int barWidth = 40;
        int pos = (int)(barWidth * progress / 100.0f);
        for (int i = 0; i < barWidth; i++) {
            if (i < pos) printf("=");
            else if (i == pos) printf(">");
            else printf(" ");
        }
        printf("] %.1f%%", progress);

        printf(" | %lld/%lld comparaisons", (long long)completed, monitor->totalComparisons);
        printf(" | Temps: %.1fs", elapsedSeconds);

        if (remainingSeconds > 0 && progress < 99.0f) {
            printf(" | Reste: ~%.1fs", remainingSeconds);
        }

        printf("        \r");  // Espaces pour effacer l'ancien texte
        fflush(stdout);

        Sleep(100);  // Update toutes les 100ms
    }

    // Barre finale complete
    COORD coord;
    coord.X = 0;
    coord.Y = startY;
    SetConsoleCursorPosition(hConsole, coord);

    DWORD elapsed = GetTickCount() - monitor->startTime;
    printf("  [========================================] 100.0%%");
    printf(" | %lld comparaisons", monitor->totalComparisons);
    printf(" | Temps: %.3fs", elapsed / 1000.0f);
    printf("        \n");

    return 0;
}

// VERSION SINGLE-THREADED
int DetectCollisionsSingleThreaded(Sphere* spheres, int count, CollisionResult* result, ProgressMonitor* progress) {
    int collisions = 0;

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (CheckCollision(spheres[i], spheres[j])) {
                result->AddCollision(spheres[i].id, spheres[j].id);
                collisions++;
            }

            // Incrementer la progression
            if (progress != NULL) {
                progress->IncrementProgress();
            }
        }
    }

    return collisions;
}

// Thread worker pour detection parallele
DWORD WINAPI CollisionWorkerThread(LPVOID param) {
    ThreadWorkParams* params = (ThreadWorkParams*)param;

    int localCollisions = 0;

    // Chaque thread traite une plage de spheres
    for (int i = params->startIndex; i < params->endIndex; i++) {
        for (int j = i + 1; j < params->totalSpheres; j++) {
            if (CheckCollision(params->spheres[i], params->spheres[j])) {
                params->result->AddCollision(params->spheres[i].id, params->spheres[j].id);
                localCollisions++;
            }

            // Incrementer la progression
            if (params->progress != NULL) {
                params->progress->IncrementProgress();
            }
        }
    }

    return localCollisions;
}

// VERSION MULTI-THREADED
int DetectCollisionsMultiThreaded(Sphere* spheres, int count, CollisionResult* result, int numThreads, ProgressMonitor* progress) {
    if (numThreads <= 0) numThreads = 1;
    if (numThreads > count) numThreads = count;

    HANDLE* threads = new HANDLE[numThreads];
    ThreadWorkParams* params = new ThreadWorkParams[numThreads];

    int spheresPerThread = count / numThreads;
    int remainingSpheres = count % numThreads;

    // Creer les threads
    int startIdx = 0;
    for (int i = 0; i < numThreads; i++) {
        int endIdx = startIdx + spheresPerThread;
        if (i == numThreads - 1) {
            endIdx += remainingSpheres;
        }

        params[i].spheres = spheres;
        params[i].totalSpheres = count;
        params[i].startIndex = startIdx;
        params[i].endIndex = endIdx;
        params[i].result = result;
        params[i].progress = progress;

        threads[i] = CreateThread(
            NULL,
            0,
            CollisionWorkerThread,
            &params[i],
            0,
            NULL
        );

        startIdx = endIdx;
    }

    // Attendre tous les threads
    WaitForMultipleObjects(numThreads, threads, TRUE, INFINITE);

    // Nettoyer
    for (int i = 0; i < numThreads; i++) {
        CloseHandle(threads[i]);
    }

    delete[] threads;
    delete[] params;

    return result->collisionCount;
}
