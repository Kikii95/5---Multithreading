#ifndef COLLISIONDETECTOR_H
#define COLLISIONDETECTOR_H

#include <windows.h>
#include "Sphere.h"

struct CollisionPair {
    int sphereId1;
    int sphereId2;
};

struct ProgressMonitor {
    volatile LONG comparisonsCompleted;
    long long totalComparisons;
    volatile LONG running;
    DWORD startTime;
    HANDLE monitorThread;

    ProgressMonitor(long long total) : comparisonsCompleted(0), totalComparisons(total), running(0), startTime(0), monitorThread(NULL) {}

    void Start() {
        InterlockedExchange(&running, 1);
        startTime = GetTickCount();
        DWORD threadId;
        monitorThread = CreateThread(NULL, 0, ProgressMonitorThread, this, 0, &threadId);
    }

    void Stop() {
        InterlockedExchange(&running, 0);
        if (monitorThread != NULL) {
            WaitForSingleObject(monitorThread, INFINITE);
            CloseHandle(monitorThread);
        }
    }

    void IncrementProgress() {
        InterlockedIncrement(&comparisonsCompleted);
    }
};

DWORD WINAPI ProgressMonitorThread(LPVOID param);

struct CollisionResult {
    CollisionPair* collisions;
    int collisionCount;
    int maxCollisions;
    CRITICAL_SECTION cs;

    CollisionResult(int maxColl) : collisionCount(0), maxCollisions(maxColl) {
        collisions = new CollisionPair[maxCollisions];
        InitializeCriticalSection(&cs);
    }

    ~CollisionResult() {
        delete[] collisions;
        DeleteCriticalSection(&cs);
    }

    void AddCollision(int id1, int id2) {
        EnterCriticalSection(&cs);
        if (collisionCount < maxCollisions) {
            collisions[collisionCount].sphereId1 = id1;
            collisions[collisionCount].sphereId2 = id2;
            collisionCount++;
        }
        LeaveCriticalSection(&cs);
    }
};

struct ThreadWorkParams {
    Sphere* spheres;
    int totalSpheres;
    int startIndex;
    int endIndex;
    CollisionResult* result;
    ProgressMonitor* progress;
};

int DetectCollisionsSingleThreaded(Sphere* spheres, int count, CollisionResult* result, ProgressMonitor* progress = NULL);

int DetectCollisionsMultiThreaded(Sphere* spheres, int count, CollisionResult* result, int numThreads, ProgressMonitor* progress = NULL);

DWORD WINAPI CollisionWorkerThread(LPVOID param);

#endif // COLLISIONDETECTOR_H
