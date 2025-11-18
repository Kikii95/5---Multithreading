#include "Game.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

DWORD WINAPI SpawnerThreadFunc(LPVOID param) {
    Game* game = (Game*)param;

    while (InterlockedCompareExchange(&game->running, 0, 0) == 1) {
        SpawnMole(game);
        Sleep(game->spawnIntervalMs);
        CleanupDeadMoles(game);
    }

    return 0;
}

Game* CreateGame(int difficulty) {
    Game* game = (Game*)malloc(sizeof(Game));
    if (game == NULL) return NULL;

    memset(game, 0, sizeof(Game));

    game->difficulty = difficulty;
    game->score = 0;
    game->running = 0;
    game->nextMoleId = 1;

    // Create message queue for thread communication
    game->messageQueue = CreateMessageQueue();
    if (game->messageQueue == NULL) {
        free(game);
        return NULL;
    }

    switch (difficulty) {
    case 1: // Easy
        game->spawnIntervalMs = 2000;
        game->minLifeMs = 2000;
        game->maxLifeMs = 5000;
        break;
    case 2: // Medium
        game->spawnIntervalMs = 1000;
        game->minLifeMs = 1000;
        game->maxLifeMs = 3500;
        break;
    case 3: // Hard
        game->spawnIntervalMs = 500;
        game->minLifeMs = 500;
        game->maxLifeMs = 2000;
        break;
    default:
        game->spawnIntervalMs = 1000;
        game->minLifeMs = 500;
        game->maxLifeMs = 5000;
        break;
    }

    return game;
}

BOOL StartGame(Game* game) {
    InterlockedExchange(&game->running, 1);

    DWORD threadId;
    game->spawnerThread = CreateThread(
        NULL,
        0,
        SpawnerThreadFunc,
        game,
        0,
        &threadId
    );

    if (game->spawnerThread == NULL) {
        DWORD error = GetLastError();
        printf("ERROR: Failed to create spawner thread! Error code: %lu\n", error);
        InterlockedExchange(&game->running, 0);
        return FALSE;
    }

    return TRUE;
}

void StopGame(Game* game) {
    InterlockedExchange(&game->running, 0);

    if (game->spawnerThread != NULL) {
        WaitForSingleObject(game->spawnerThread, INFINITE);
        CloseHandle(game->spawnerThread);
    }

    for (int i = 0; i < game->moleCount; i++) {
        if (game->moles[i] != NULL) {
            WaitMole(game->moles[i]);
        }
    }
}

void SpawnMole(Game* game) {
    if (game->moleCount >= MAX_MOLES) {
        return;
    }

    char key = RandomKey();
    int lifetime = RandomRange(game->minLifeMs, game->maxLifeMs);

    Mole* mole = CreateMole(game->nextMoleId++, key, lifetime, game->messageQueue);
    if (mole == NULL) return;

    game->moles[game->moleCount++] = mole;

    // Push spawn message
    Message msg(MSG_MOLE_SPAWNED, mole->id, mole->key);
    TryPushMessage(game->messageQueue, msg);
}

BOOL TryKillMole(Game* game, char key, int* outMoleId) {
    if (key >= 'a' && key <= 'z') {
        key = key - 'a' + 'A';
    }

    for (int i = 0; i < game->moleCount; i++) {
        Mole* mole = game->moles[i];
        if (mole != NULL && mole->key == key && IsMoleAlive(mole)) {
            if (KillMole(mole)) {
                if (outMoleId != NULL) {
                    *outMoleId = mole->id;
                }
                InterlockedIncrement(&game->score);
                return TRUE;
            }
        }
    }

    InterlockedDecrement(&game->score);
    return FALSE;
}

int GetScore(Game* game) {
    return InterlockedCompareExchange(&game->score, 0, 0);
}

int GetLivingMoles(Game* game) {
    int count = 0;
    for (int i = 0; i < game->moleCount; i++) {
        if (game->moles[i] != NULL && IsMoleAlive(game->moles[i])) {
            count++;
        }
    }
    return count;
}

void CleanupDeadMoles(Game* game) {
    int writeIndex = 0;
    for (int readIndex = 0; readIndex < game->moleCount; readIndex++) {
        Mole* mole = game->moles[readIndex];

        if (mole != NULL && !IsMoleAlive(mole)) {
            if (InterlockedCompareExchange(&mole->killed, 0, 0) == 0) {
                InterlockedDecrement(&game->score);
            }

            WaitMole(mole);
            DestroyMole(mole);
        }
        else {
            game->moles[writeIndex++] = mole;
        }
    }

    game->moleCount = writeIndex;
}

void DestroyGame(Game* game) {
    if (game == NULL) return;

    StopGame(game);

    for (int i = 0; i < game->moleCount; i++) {
        if (game->moles[i] != NULL) {
            DestroyMole(game->moles[i]);
        }
    }

    // Destroy message queue
    if (game->messageQueue != NULL) {
        DestroyMessageQueue(game->messageQueue);
    }

    free(game);
}
