#include "SafeCollisionDetector.h"
#include <stdio.h>

DWORD WINAPI ProgressMonitorThread(LPVOID param) {
    ProgressMonitor* monitor = (ProgressMonitor*)param;

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    int startY = csbi.dwCursorPosition.Y;

    while (InterlockedCompareExchange(&monitor->running, 0, 0) == 1) {
        long long completed = monitor->comparisonsCompleted; 
        float progress = (float)completed / monitor->totalComparisons * 100.0f;

        if (progress > 100.0f) progress = 100.0f;

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
                    InterlockedExchangeAdd64((volatile LONGLONG*)&progress->comparisonsCompleted, PROGRESS_BATCH);
                    progressCounter = 0;
                }
            }
        }
    }

    if (progress != NULL && progressCounter > 0) {
        InterlockedExchangeAdd64((volatile LONGLONG*)&progress->comparisonsCompleted, progressCounter);
    }

    return collisions;
}

DWORD WINAPI CollisionProducerThread(LPVOID param) {
    ProducerThreadParams* params = (ProducerThreadParams*)param;

    int localCollisions = 0;
    int progressCounter = 0;
    const int PROGRESS_BATCH = 10000;

    for (int i = params->startIndex; i < params->endIndex; i++) {
        if (InterlockedCompareExchange(params->stopProduction, 0, 0) == 1) {
            break; 
        }

        for (int j = i + 1; j < params->totalSpheres; j++) {
            if (InterlockedCompareExchange(params->stopProduction, 0, 0) == 1) {
                break;
            }

            if (CheckCollision(params->spheres[i], params->spheres[j])) {
                CollisionPair collision(params->spheres[i].id, params->spheres[j].id);
                params->queue->push(collision);

                InterlockedIncrement(params->collisionCount);
                localCollisions++;
            }

            if (params->progress != NULL) {
                progressCounter++;
                if (progressCounter >= PROGRESS_BATCH) {
                    InterlockedExchangeAdd64((volatile LONGLONG*)&params->progress->comparisonsCompleted, PROGRESS_BATCH);
                    progressCounter = 0;
                }
            }
        }

        if (InterlockedCompareExchange(params->stopProduction, 0, 0) == 1) {
            break;
        }
    }

    if (params->progress != NULL && progressCounter > 0) {
        InterlockedExchangeAdd64((volatile LONGLONG*)&params->progress->comparisonsCompleted, progressCounter);
    }

    return localCollisions;
}

DWORD WINAPI CollisionConsumerThread(LPVOID param) {
    ConsumerThreadParams* params = (ConsumerThreadParams*)param;

    int localCount = 0;

    while (InterlockedCompareExchange(params->running, 0, 0) == 1 || !params->queue->isEmpty()) {
        CollisionPair collision;

        EnterCriticalSection(params->cs);
        bool bufferFull = (*params->resultCount >= params->maxResults);
        LeaveCriticalSection(params->cs);

        if (bufferFull) {
            InterlockedExchange(params->stopProduction, 1);
            break;
        }

        if (params->queue->tryPop(collision)) {
            EnterCriticalSection(params->cs);
            if (*params->resultCount < params->maxResults) {
                params->resultBuffer[*params->resultCount] = collision;
                (*params->resultCount)++;
                localCount++;
            }
            LeaveCriticalSection(params->cs);
        } else {
            Sleep(1);
        }
    }

    return localCount;
}

DWORD WINAPI CollisionConsumerThreadOptimized(LPVOID param) {
    ConsumerThreadParams* params = (ConsumerThreadParams*)param;

    const int LOCAL_BUFFER_SIZE = 10000;
    CollisionPair* localBuffer = new CollisionPair[LOCAL_BUFFER_SIZE];
    int localCount = 0;
    int totalProcessed = 0;

    while (InterlockedCompareExchange(params->running, 0, 0) == 1 || !params->queue->isEmpty()) {
        EnterCriticalSection(params->cs);
        bool bufferFull = (*params->resultCount >= params->maxResults);
        LeaveCriticalSection(params->cs);

        if (bufferFull) {
            InterlockedExchange(params->stopProduction, 1);
            break;
        }

        CollisionPair collision;

        if (params->queue->tryPop(collision)) {
            localBuffer[localCount++] = collision;
            totalProcessed++;

            if (localCount >= LOCAL_BUFFER_SIZE) {
                EnterCriticalSection(params->cs);
                if (*params->resultCount + localCount <= params->maxResults) {
                    memcpy(&params->resultBuffer[*params->resultCount],
                           localBuffer,
                           localCount * sizeof(CollisionPair));
                    *params->resultCount += localCount;
                } else {
                    int remaining = params->maxResults - *params->resultCount;
                    if (remaining > 0) {
                        memcpy(&params->resultBuffer[*params->resultCount],
                               localBuffer,
                               remaining * sizeof(CollisionPair));
                        *params->resultCount += remaining;
                    }
                }
                LeaveCriticalSection(params->cs);
                localCount = 0;  
            }
        } else {
            Sleep(1);
        }
    }

    if (localCount > 0) {
        EnterCriticalSection(params->cs);
        if (*params->resultCount + localCount <= params->maxResults) {
            memcpy(&params->resultBuffer[*params->resultCount],
                   localBuffer,
                   localCount * sizeof(CollisionPair));
            *params->resultCount += localCount;
        } else {
            int remaining = params->maxResults - *params->resultCount;
            if (remaining > 0) {
                memcpy(&params->resultBuffer[*params->resultCount],
                       localBuffer,
                       remaining * sizeof(CollisionPair));
                *params->resultCount += remaining;
            }
        }
        LeaveCriticalSection(params->cs);
    }

    delete[] localBuffer;
    return totalProcessed;
}

int DetectCollisionsWithSafeQueue(Sphere* spheres, int count, CollisionPair* results, int maxResults,
                                   int numProducers, int numConsumers, ProgressMonitor* progress) {
    if (numProducers <= 0) numProducers = 1;
    if (numConsumers <= 0) numConsumers = 1;
    if (numProducers > count) numProducers = count;

    CollisionQueue queue;

    volatile LONG collisionCount = 0;

    int resultCount = 0;
    CRITICAL_SECTION resultCS;
    InitializeCriticalSection(&resultCS);
    volatile LONG producersRunning = numProducers;
    volatile LONG stopProduction = 0; 

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
        producerParams[i].stopProduction = &stopProduction; 

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

    HANDLE* consumers = new HANDLE[numConsumers];
    ConsumerThreadParams* consumerParams = new ConsumerThreadParams[numConsumers];

    for (int i = 0; i < numConsumers; i++) {
        consumerParams[i].queue = &queue;
        consumerParams[i].resultBuffer = results;
        consumerParams[i].resultCount = &resultCount;
        consumerParams[i].maxResults = maxResults;
        consumerParams[i].running = &producersRunning;
        consumerParams[i].cs = &resultCS;
        consumerParams[i].stopProduction = &stopProduction; 

        consumers[i] = CreateThread(
            NULL,
            0,
            CollisionConsumerThread,
            &consumerParams[i],
            0,
            NULL
        );
    }

    WaitForMultipleObjects(numProducers, producers, TRUE, INFINITE);
    InterlockedExchange(&producersRunning, 0);

    WaitForMultipleObjects(numConsumers, consumers, TRUE, INFINITE);

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

int DetectCollisionsWithSafeQueueOptimized(Sphere* spheres, int count, CollisionPair* results, int maxResults,
                                            int numProducers, int numConsumers, ProgressMonitor* progress) {
    if (numProducers <= 0) numProducers = 1;
    if (numConsumers <= 0) numConsumers = 1;
    if (numProducers > count) numProducers = count;

    CollisionQueue queue;

    volatile LONG collisionCount = 0;

    int resultCount = 0;
    CRITICAL_SECTION resultCS;
    InitializeCriticalSection(&resultCS);
    volatile LONG producersRunning = numProducers;
    volatile LONG stopProduction = 0;  

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
        producerParams[i].stopProduction = &stopProduction; 

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

    HANDLE* consumers = new HANDLE[numConsumers];
    ConsumerThreadParams* consumerParams = new ConsumerThreadParams[numConsumers];

    for (int i = 0; i < numConsumers; i++) {
        consumerParams[i].queue = &queue;
        consumerParams[i].resultBuffer = results;
        consumerParams[i].resultCount = &resultCount;
        consumerParams[i].maxResults = maxResults;
        consumerParams[i].running = &producersRunning;
        consumerParams[i].cs = &resultCS;
        consumerParams[i].stopProduction = &stopProduction; 

        consumers[i] = CreateThread(
            NULL,
            0,
            CollisionConsumerThreadOptimized,  
            &consumerParams[i],
            0,
            NULL
        );
    }

    WaitForMultipleObjects(numProducers, producers, TRUE, INFINITE);
    InterlockedExchange(&producersRunning, 0); 

    WaitForMultipleObjects(numConsumers, consumers, TRUE, INFINITE);

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
