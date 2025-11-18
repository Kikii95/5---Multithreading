#ifndef GAME_H
#define GAME_H

#include <windows.h>
#include "Mole.h"
#include "GameQueue.h"

#define MAX_MOLES 100

typedef struct {
    Mole* moles[MAX_MOLES];
    int moleCount;
    volatile LONG score;
    volatile LONG running;
    HANDLE spawnerThread;
    GameMessageQueue messageQueue;  // SafeQueue for thread communication

    int difficulty;
    int spawnIntervalMs;
    int minLifeMs;
    int maxLifeMs;
    int nextMoleId;
} Game;

DWORD WINAPI SpawnerThreadFunc(LPVOID param);

Game* CreateGame(int difficulty);

BOOL StartGame(Game* game);

void StopGame(Game* game);

void SpawnMole(Game* game);

BOOL TryKillMole(Game* game, char key, int* outMoleId);

int GetScore(Game* game);

int GetLivingMoles(Game* game);

void CleanupDeadMoles(Game* game);

void DestroyGame(Game* game);

#endif // GAME_H
