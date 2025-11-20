#ifndef SAFECOLLISIONDETECTOR_H
#define SAFECOLLISIONDETECTOR_H

#include <windows.h>
#include "Sphere.h"
#include "SafeQueue.h"

// Structure pour stocker une collision detectee
struct CollisionPair {
    int sphereId1;
    int sphereId2;

    CollisionPair() : sphereId1(0), sphereId2(0) {}
    CollisionPair(int id1, int id2) : sphereId1(id1), sphereId2(id2) {}
};

// Forward declaration
DWORD WINAPI ProgressMonitorThread(LPVOID param);

// Structure pour suivre la progression
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

// Type de la SafeQueue pour les collisions
typedef SafeQueue<CollisionPair, 100000> CollisionQueue;

// Structure pour les parametres de thread producer
struct ProducerThreadParams {
    Sphere* spheres;
    int totalSpheres;
    int startIndex;
    int endIndex;
    CollisionQueue* queue;
    ProgressMonitor* progress;
    volatile LONG* collisionCount;  // Compteur partagé
};

// Structure pour les parametres de thread consumer
struct ConsumerThreadParams {
    CollisionQueue* queue;
    CollisionPair* resultBuffer;
    int* resultCount;
    int maxResults;
    volatile LONG* running;
    CRITICAL_SECTION* cs;
};

// SINGLE-THREADED : Detection de collisions simple
int DetectCollisionsSingleThreaded(Sphere* spheres, int count, CollisionPair* results, int maxResults, ProgressMonitor* progress = NULL);

// MULTI-THREADED avec SafeQueue : Producers + Consumers
int DetectCollisionsWithSafeQueue(Sphere* spheres, int count, CollisionPair* results, int maxResults,
                                   int numProducers, int numConsumers, ProgressMonitor* progress = NULL);

// MULTI-THREADED avec SafeQueue + ACCUMULATION LOCALE (optimisé)
int DetectCollisionsWithSafeQueueOptimized(Sphere* spheres, int count, CollisionPair* results, int maxResults,
                                            int numProducers, int numConsumers, ProgressMonitor* progress = NULL);

// Thread producer (detecte collisions et les envoie dans la queue)
DWORD WINAPI CollisionProducerThread(LPVOID param);

// Thread consumer (recupere collisions de la queue et les stocke)
DWORD WINAPI CollisionConsumerThread(LPVOID param);

// Thread consumer OPTIMISE (avec accumulation locale)
DWORD WINAPI CollisionConsumerThreadOptimized(LPVOID param);

// Thread de monitoring de progression
DWORD WINAPI ProgressMonitorThread(LPVOID param);

#endif // SAFECOLLISIONDETECTOR_H
