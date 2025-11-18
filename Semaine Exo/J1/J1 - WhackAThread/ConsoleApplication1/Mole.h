#ifndef MOLE_H
#define MOLE_H

#include <windows.h>
#include "GameQueue.h"

typedef struct {
    int id;
    char key;
    int lifetimeMs;
    volatile LONG alive;
    volatile LONG killed;
    HANDLE thread;
    DWORD startTime;
    GameMessageQueue messageQueue;  // Pointer to game message queue
} Mole;

DWORD WINAPI MoleThreadFunc(LPVOID param);

Mole* CreateMole(int id, char key, int lifetimeMs, GameMessageQueue queue);

BOOL IsMoleAlive(Mole* mole);

BOOL KillMole(Mole* mole);

void WaitMole(Mole* mole);

void DestroyMole(Mole* mole);

#endif // MOLE_H
