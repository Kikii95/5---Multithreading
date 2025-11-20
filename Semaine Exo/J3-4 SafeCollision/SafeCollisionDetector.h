#ifndef SAFECOLLISIONDETECTOR_H
#define SAFECOLLISIONDETECTOR_H

#include <windows.h>
#include "Sphere.h"
#include "SafeQueue.h"

struct CollisionPair {
    int sphereId1;
    int sphereId2;

    CollisionPair() : sphereId1(0), sphereId2(0) {}
    CollisionPair(int id1, int id2) : sphereId1(id1), sphereId2(id2) {}
};

DWORD WINAPI ProgressMonitorThread(LPVOID param);

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
};

typedef SafeQueue<CollisionPair, 100000> CollisionQueue;

struct ProducerThreadParams {
    Sphere* spheres;
    int totalSpheres;
    int startIndex;
    int endIndex;
    CollisionQueue* queue;
    ProgressMonitor* progress;
    volatile LONG* collisionCount;  
};

struct ConsumerThreadParams {
    CollisionQueue* queue;
    CollisionPair* resultBuffer;
    int* resultCount;
    int maxResults;
    volatile LONG* running;
    CRITICAL_SECTION* cs;
};

int DetectCollisionsSingleThreaded(Sphere* spheres, int count, CollisionPair* results, int maxResults, ProgressMonitor* progress = NULL);

int DetectCollisionsWithSafeQueue(Sphere* spheres, int count, CollisionPair* results, int maxResults,
                                   int numProducers, int numConsumers, ProgressMonitor* progress = NULL);

int DetectCollisionsWithSafeQueueOptimized(Sphere* spheres, int count, CollisionPair* results, int maxResults,
                                            int numProducers, int numConsumers, ProgressMonitor* progress = NULL);

DWORD WINAPI CollisionProducerThread(LPVOID param);

DWORD WINAPI CollisionConsumerThread(LPVOID param);

DWORD WINAPI CollisionConsumerThreadOptimized(LPVOID param);

DWORD WINAPI ProgressMonitorThread(LPVOID param);

#endif // SAFECOLLISIONDETECTOR_H
