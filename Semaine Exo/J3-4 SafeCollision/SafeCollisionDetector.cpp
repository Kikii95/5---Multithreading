#include "SafeCollisionDetector.h"
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

        float remainingSeconds = 0;
        if (completed > 0) {
            float timePerComparison = elapsedSeconds / completed;
            remainingSeconds = timePerComparison * (monitor->totalComparisons - completed);
        }

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

        printf("        \r");
        fflush(stdout);

        Sleep(100);
    }

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

// VERSION SINGLE-THREADED (reference)
int DetectCollisionsSingleThreaded(Sphere* spheres, int count, CollisionPair* results, int maxResults, ProgressMonitor* progress) {
    int collisions = 0;
    int progressCounter = 0;
    const int PROGRESS_BATCH = 10000;

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (CheckCollision(spheres[i], spheres[j])) {
                if (collisions < maxResults) {
                    results[collisions] = CollisionPair(spheres[i].id, spheres[j].id);
                    collisions++;
                }
            }

            if (progress != NULL) {
                progressCounter++;
                if (progressCounter >= PROGRESS_BATCH) {
                    InterlockedExchangeAdd(&progress->comparisonsCompleted, PROGRESS_BATCH);
                    progressCounter = 0;
                }
            }
        }
    }

    if (progress != NULL && progressCounter > 0) {
        InterlockedExchangeAdd(&progress->comparisonsCompleted, progressCounter);
    }

    return collisions;
}

// Thread PRODUCER : Detecte collisions et les envoie dans la SafeQueue
DWORD WINAPI CollisionProducerThread(LPVOID param) {
    ProducerThreadParams* params = (ProducerThreadParams*)param;

    int localCollisions = 0;
    int progressCounter = 0;
    const int PROGRESS_BATCH = 10000;

    for (int i = params->startIndex; i < params->endIndex; i++) {
        for (int j = i + 1; j < params->totalSpheres; j++) {
            if (CheckCollision(params->spheres[i], params->spheres[j])) {
                // Envoyer la collision dans la SafeQueue
                CollisionPair collision(params->spheres[i].id, params->spheres[j].id);
                params->queue->push(collision);  // Bloquant si queue pleine

                InterlockedIncrement(params->collisionCount);
                localCollisions++;
            }

            if (params->progress != NULL) {
                progressCounter++;
                if (progressCounter >= PROGRESS_BATCH) {
                    InterlockedExchangeAdd(&params->progress->comparisonsCompleted, PROGRESS_BATCH);
                    progressCounter = 0;
                }
            }
        }
    }

    if (params->progress != NULL && progressCounter > 0) {
        InterlockedExchangeAdd(&params->progress->comparisonsCompleted, progressCounter);
    }

    return localCollisions;
}

// Thread CONSUMER : Recupere collisions de la SafeQueue et les stocke
DWORD WINAPI CollisionConsumerThread(LPVOID param) {
    ConsumerThreadParams* params = (ConsumerThreadParams*)param;

    int localCount = 0;

    while (InterlockedCompareExchange(params->running, 0, 0) == 1 || !params->queue->isEmpty()) {
        CollisionPair collision;

        // Essayer de recuperer une collision (non-bloquant)
        if (params->queue->tryPop(collision)) {
            // Stocker dans le buffer de resultats
            EnterCriticalSection(params->cs);
            if (*params->resultCount < params->maxResults) {
                params->resultBuffer[*params->resultCount] = collision;
                (*params->resultCount)++;
                localCount++;
            }
            LeaveCriticalSection(params->cs);
        } else {
            // Queue vide, attendre un peu
            Sleep(1);
        }
    }

    return localCount;
}

// Thread CONSUMER OPTIMISE : Avec accumulation locale (buffer local)
DWORD WINAPI CollisionConsumerThreadOptimized(LPVOID param) {
    ConsumerThreadParams* params = (ConsumerThreadParams*)param;

    // Buffer local pour accumuler les collisions avant flush
    const int LOCAL_BUFFER_SIZE = 10000;
    CollisionPair* localBuffer = new CollisionPair[LOCAL_BUFFER_SIZE];
    int localCount = 0;
    int totalProcessed = 0;

    while (InterlockedCompareExchange(params->running, 0, 0) == 1 || !params->queue->isEmpty()) {
        CollisionPair collision;

        // Essayer de recuperer une collision (non-bloquant)
        if (params->queue->tryPop(collision)) {
            // Stocker dans le buffer LOCAL (PAS de lock !)
            localBuffer[localCount++] = collision;
            totalProcessed++;

            // Si buffer local plein, flush vers result buffer
            if (localCount >= LOCAL_BUFFER_SIZE) {
                EnterCriticalSection(params->cs);
                if (*params->resultCount + localCount <= params->maxResults) {
                    // Copie en bloc (memcpy rapide)
                    memcpy(&params->resultBuffer[*params->resultCount],
                           localBuffer,
                           localCount * sizeof(CollisionPair));
                    *params->resultCount += localCount;
                }
                LeaveCriticalSection(params->cs);
                localCount = 0;  // Reset buffer local
            }
        } else {
            // Queue vide, attendre un peu
            Sleep(1);
        }
    }

    // Flush final : items restants dans buffer local
    if (localCount > 0) {
        EnterCriticalSection(params->cs);
        if (*params->resultCount + localCount <= params->maxResults) {
            memcpy(&params->resultBuffer[*params->resultCount],
                   localBuffer,
                   localCount * sizeof(CollisionPair));
            *params->resultCount += localCount;
        }
        LeaveCriticalSection(params->cs);
    }

    delete[] localBuffer;
    return totalProcessed;
}

// VERSION MULTI-THREADED avec SafeQueue (Producer-Consumer)
int DetectCollisionsWithSafeQueue(Sphere* spheres, int count, CollisionPair* results, int maxResults,
                                   int numProducers, int numConsumers, ProgressMonitor* progress) {
    if (numProducers <= 0) numProducers = 1;
    if (numConsumers <= 0) numConsumers = 1;
    if (numProducers > count) numProducers = count;

    // Creer la SafeQueue
    CollisionQueue queue;

    // Compteur de collisions
    volatile LONG collisionCount = 0;

    // Variables pour les consumers
    int resultCount = 0;
    CRITICAL_SECTION resultCS;
    InitializeCriticalSection(&resultCS);
    volatile LONG producersRunning = numProducers;

    // Creer les threads PRODUCERS
    HANDLE* producers = new HANDLE[numProducers];
    ProducerThreadParams* producerParams = new ProducerThreadParams[numProducers];

    int spheresPerProducer = count / numProducers;
    int remainingSpheres = count % numProducers;

    int startIdx = 0;
    for (int i = 0; i < numProducers; i++) {
        int endIdx = startIdx + spheresPerProducer;
        if (i == numProducers - 1) {
            endIdx += remainingSpheres;
        }

        producerParams[i].spheres = spheres;
        producerParams[i].totalSpheres = count;
        producerParams[i].startIndex = startIdx;
        producerParams[i].endIndex = endIdx;
        producerParams[i].queue = &queue;
        producerParams[i].progress = progress;
        producerParams[i].collisionCount = &collisionCount;

        producers[i] = CreateThread(
            NULL,
            0,
            CollisionProducerThread,
            &producerParams[i],
            0,
            NULL
        );

        startIdx = endIdx;
    }

    // Creer les threads CONSUMERS
    HANDLE* consumers = new HANDLE[numConsumers];
    ConsumerThreadParams* consumerParams = new ConsumerThreadParams[numConsumers];

    for (int i = 0; i < numConsumers; i++) {
        consumerParams[i].queue = &queue;
        consumerParams[i].resultBuffer = results;
        consumerParams[i].resultCount = &resultCount;
        consumerParams[i].maxResults = maxResults;
        consumerParams[i].running = &producersRunning;
        consumerParams[i].cs = &resultCS;

        consumers[i] = CreateThread(
            NULL,
            0,
            CollisionConsumerThread,
            &consumerParams[i],
            0,
            NULL
        );
    }

    // Attendre que tous les PRODUCERS finissent
    WaitForMultipleObjects(numProducers, producers, TRUE, INFINITE);
    InterlockedExchange(&producersRunning, 0);  // Signaler aux consumers

    // Attendre que tous les CONSUMERS finissent
    WaitForMultipleObjects(numConsumers, consumers, TRUE, INFINITE);

    // Nettoyer
    for (int i = 0; i < numProducers; i++) {
        CloseHandle(producers[i]);
    }
    for (int i = 0; i < numConsumers; i++) {
        CloseHandle(consumers[i]);
    }

    delete[] producers;
    delete[] producerParams;
    delete[] consumers;
    delete[] consumerParams;

    DeleteCriticalSection(&resultCS);

    return resultCount;
}

// VERSION MULTI-THREADED avec SafeQueue + ACCUMULATION LOCALE (optimisé)
int DetectCollisionsWithSafeQueueOptimized(Sphere* spheres, int count, CollisionPair* results, int maxResults,
                                            int numProducers, int numConsumers, ProgressMonitor* progress) {
    if (numProducers <= 0) numProducers = 1;
    if (numConsumers <= 0) numConsumers = 1;
    if (numProducers > count) numProducers = count;

    // Creer la SafeQueue
    CollisionQueue queue;

    // Compteur de collisions
    volatile LONG collisionCount = 0;

    // Variables pour les consumers
    int resultCount = 0;
    CRITICAL_SECTION resultCS;
    InitializeCriticalSection(&resultCS);
    volatile LONG producersRunning = numProducers;

    // Creer les threads PRODUCERS (identique à la version non-optimisée)
    HANDLE* producers = new HANDLE[numProducers];
    ProducerThreadParams* producerParams = new ProducerThreadParams[numProducers];

    int spheresPerProducer = count / numProducers;
    int remainingSpheres = count % numProducers;

    int startIdx = 0;
    for (int i = 0; i < numProducers; i++) {
        int endIdx = startIdx + spheresPerProducer;
        if (i == numProducers - 1) {
            endIdx += remainingSpheres;
        }

        producerParams[i].spheres = spheres;
        producerParams[i].totalSpheres = count;
        producerParams[i].startIndex = startIdx;
        producerParams[i].endIndex = endIdx;
        producerParams[i].queue = &queue;
        producerParams[i].progress = progress;
        producerParams[i].collisionCount = &collisionCount;

        producers[i] = CreateThread(
            NULL,
            0,
            CollisionProducerThread,
            &producerParams[i],
            0,
            NULL
        );

        startIdx = endIdx;
    }

    // Creer les threads CONSUMERS OPTIMISES (avec accumulation locale)
    HANDLE* consumers = new HANDLE[numConsumers];
    ConsumerThreadParams* consumerParams = new ConsumerThreadParams[numConsumers];

    for (int i = 0; i < numConsumers; i++) {
        consumerParams[i].queue = &queue;
        consumerParams[i].resultBuffer = results;
        consumerParams[i].resultCount = &resultCount;
        consumerParams[i].maxResults = maxResults;
        consumerParams[i].running = &producersRunning;
        consumerParams[i].cs = &resultCS;

        consumers[i] = CreateThread(
            NULL,
            0,
            CollisionConsumerThreadOptimized,  // ← Version optimisée
            &consumerParams[i],
            0,
            NULL
        );
    }

    // Attendre que tous les PRODUCERS finissent
    WaitForMultipleObjects(numProducers, producers, TRUE, INFINITE);
    InterlockedExchange(&producersRunning, 0);  // Signaler aux consumers

    // Attendre que tous les CONSUMERS finissent
    WaitForMultipleObjects(numConsumers, consumers, TRUE, INFINITE);

    // Nettoyer
    for (int i = 0; i < numProducers; i++) {
        CloseHandle(producers[i]);
    }
    for (int i = 0; i < numConsumers; i++) {
        CloseHandle(consumers[i]);
    }

    delete[] producers;
    delete[] producerParams;
    delete[] consumers;
    delete[] consumerParams;

    DeleteCriticalSection(&resultCS);

    return resultCount;
}
