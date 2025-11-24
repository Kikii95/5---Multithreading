#pragma once
#include <vector>
#include "WorkerThread.h"
#include "../Core/Utils.h"

class ThreadManager {
private:
    std::vector<WorkerThread*> mThreads;
    int mGridWidth; 
    int mGridHeight;  
    float mCellSize;

    bool mGameOver;
    int mWinnerID;

public:
    ThreadManager(int nbThreads, int gridWidth, int gridHeight, float cellSize);
    ~ThreadManager();

    void StartAll();
    void StopAll();
    void JoinAll();

    WorkerThread* GetThread(int index);
    std::vector<Entity*> GetAllEntities();
    int GetAliveCount();

    int CalculateThreadFromPosition(Vector2 pos);

    bool IsGameOver() const { return mGameOver; }
    int GetWinnerID() const { return mWinnerID; }
    void CheckGameOver();

    void PrintStats();

    void SpawnBalls(int count);
};
