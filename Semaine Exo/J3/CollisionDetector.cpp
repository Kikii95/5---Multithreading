#include "CollisionDetector.h"

// VERSION SINGLE-THREADED
int DetectCollisionsSingleThreaded(Sphere* spheres, int count, CollisionResult* result) {
    int collisions = 0;

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (CheckCollision(spheres[i], spheres[j])) {
                result->AddCollision(spheres[i].id, spheres[j].id);
                collisions++;
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
        }
    }

    return localCollisions;
}

// VERSION MULTI-THREADED
int DetectCollisionsMultiThreaded(Sphere* spheres, int count, CollisionResult* result, int numThreads) {
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
