#pragma once
#include <vector>
#include "WorkerThread.h"
#include "../Core/Utils.h"

// Manager global des threads
class ThreadManager {
private:
    std::vector<WorkerThread*> mThreads;
    int mGridWidth;   // Nb cellules horizontales
    int mGridHeight;  // Nb cellules verticales
    float mCellSize;  // Taille cellule

    bool mGameOver;
    int mWinnerID;

public:
    ThreadManager(int nbThreads, int gridWidth, int gridHeight, float cellSize);
    ~ThreadManager();

    // Controle threads
    void StartAll();
    void StopAll();
    void JoinAll();

    // Accesseurs
    WorkerThread* GetThread(int index);
    std::vector<Entity*> GetAllEntities();
    int GetAliveCount();

    // Calculs
    int CalculateThreadFromPosition(Vector2 pos);

    // Game state
    bool IsGameOver() const { return mGameOver; }
    int GetWinnerID() const { return mWinnerID; }
    void CheckGameOver();

    // Stats
    void PrintStats();

    // Spawn
    void SpawnBalls(int count);
};
