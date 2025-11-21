#include "ThreadManager.h"
#include "../Entity/Ball.h"
#include <stdio.h>
#include <algorithm>

ThreadManager::ThreadManager(int nbThreads, int gridWidth, int gridHeight, float cellSize)
    : mGridWidth(gridWidth), mGridHeight(gridHeight), mCellSize(cellSize),
      mGameOver(false), mWinnerID(-1) {

    for (int i = 0; i < nbThreads; i++) {
        WorkerThread* thread = new WorkerThread(i, this);
        mThreads.push_back(thread);
    }

    printf("\n=== THREAD MANAGER INITIALIZED ===\n");
    printf("Threads: %d\n", nbThreads);
    printf("Grid: %dx%d cells\n", mGridWidth, mGridHeight);
    printf("Cell size: %.1fx%.1f pixels\n", mCellSize, mCellSize);
    printf("Map size: %.1fx%.1f pixels\n", Config::MAP_WIDTH, Config::MAP_HEIGHT);
    printf("===================================\n\n");
}

ThreadManager::~ThreadManager() {
    StopAll();
    JoinAll();

    for (WorkerThread* thread : mThreads) {
        delete thread;
    }
    mThreads.clear();
}

void ThreadManager::StartAll() {
    for (WorkerThread* thread : mThreads) {
        thread->Start();
    }
    printf("[ThreadManager] All threads started\n\n");
}

void ThreadManager::StopAll() {
    for (WorkerThread* thread : mThreads) {
        thread->Stop();
    }
    printf("\n[ThreadManager] All threads stopping...\n");
}

void ThreadManager::JoinAll() {
    for (WorkerThread* thread : mThreads) {
        thread->Join();
    }
    printf("[ThreadManager] All threads joined\n");
}

WorkerThread* ThreadManager::GetThread(int index) {
    if (index >= 0 && index < (int)mThreads.size()) {
        return mThreads[index];
    }
    return nullptr;
}

std::vector<Entity*> ThreadManager::GetAllEntities() {
    std::vector<Entity*> allEntities;

    for (WorkerThread* thread : mThreads) {
        const std::vector<Entity*>& threadEntities = thread->GetEntities();
        allEntities.insert(allEntities.end(), threadEntities.begin(), threadEntities.end());
    }

    return allEntities;
}

int ThreadManager::GetAliveCount() {
    int count = 0;
    std::vector<Entity*> allEntities = GetAllEntities();
    for (Entity* e : allEntities) {
        if (!e->mIsDead) count++;
    }
    return count;
}

int ThreadManager::CalculateThreadFromPosition(Vector2 pos) {
    // Convertir position -> cellule grid
    int cellX = (int)(pos.x / mCellSize);
    int cellY = (int)(pos.y / mCellSize);

    // Clamp
    if (cellX < 0) cellX = 0;
    if (cellX >= mGridWidth) cellX = mGridWidth - 1;
    if (cellY < 0) cellY = 0;
    if (cellY >= mGridHeight) cellY = mGridHeight - 1;

    // Mapper cellule -> thread (round-robin)
    int cellIndex = cellY * mGridWidth + cellX;
    int threadIndex = cellIndex % (int)mThreads.size();

    return threadIndex;
}

void ThreadManager::CheckGameOver() {
    std::vector<Entity*> allEntities = GetAllEntities();
    std::vector<Entity*> aliveEntities;

    for (Entity* e : allEntities) {
        if (!e->mIsDead) {
            aliveEntities.push_back(e);
        }
    }

    if (aliveEntities.size() <= 1) {
        mGameOver = true;
        if (aliveEntities.size() == 1) {
            mWinnerID = aliveEntities[0]->mID;
        } else {
            mWinnerID = -1;  // Egalite (rare)
        }
    }
}

void ThreadManager::PrintStats() {
    printf("\n=== STATISTICS ===\n");

    int totalCollisions = 0;
    int totalMessages = 0;

    for (WorkerThread* thread : mThreads) {
        int collisions = thread->GetCollisionsDetected();
        int messages = thread->GetMessagesProcessed();
        int entities = (int)thread->GetEntities().size();

        printf("Thread %d: %d balls, %d collisions, %d messages\n",
               thread->GetThreadID(), entities, collisions, messages);

        totalCollisions += collisions;
        totalMessages += messages;
    }

    printf("------------------\n");
    printf("TOTAL: %d collisions, %d messages\n", totalCollisions, totalMessages);
    printf("Alive: %d balls\n", GetAliveCount());
    printf("==================\n\n");
}

void ThreadManager::SpawnBalls(int count) {
    printf("[ThreadManager] Spawning %d balls...\n", count);

    for (int i = 0; i < count; i++) {
        // Position aleatoire
        Vector2 pos(
            Utils::RandomFloat(50, Config::MAP_WIDTH - 50),
            Utils::RandomFloat(50, Config::MAP_HEIGHT - 50)
        );

        // Creer ball
        float radius = Utils::RandomFloat(15, 25);
        Ball* ball = new Ball(i, pos, radius);

        // Assigner au thread correspondant
        int threadID = CalculateThreadFromPosition(pos);
        GetThread(threadID)->AddEntity(ball);

        printf("  Ball %d spawned at (%.1f, %.1f) -> Thread %d\n",
               i, pos.x, pos.y, threadID);
    }

    printf("[ThreadManager] Spawn complete!\n\n");
}
