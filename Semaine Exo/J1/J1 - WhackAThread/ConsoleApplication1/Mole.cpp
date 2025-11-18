#include "Mole.h"
#include "utils.h"
#include "Message.h"
#include <stdlib.h>

DWORD WINAPI MoleThreadFunc(LPVOID param) {
    Mole* mole = (Mole*)param;

    DWORD startTime = GetTickCount();
    DWORD endTime = startTime + mole->lifetimeMs;

    while (GetTickCount() < endTime) {
        if (InterlockedCompareExchange(&mole->alive, 0, 0) == 0) {
            return 0;
        }

        Sleep(10);
    }

    InterlockedExchange(&mole->alive, 0);
    InterlockedExchange(&mole->killed, 0);

    if (mole->messageQueue != NULL) {
        Message msg(MSG_MOLE_EXPIRED, mole->id, mole->key);
        TryPushMessage(mole->messageQueue, msg);
    }

    return 0;
}

Mole* CreateMole(int id, char key, int lifetimeMs, GameMessageQueue queue) {
    Mole* mole = (Mole*)malloc(sizeof(Mole));
    if (mole == NULL) return NULL;

    mole->id = id;
    mole->key = key;
    mole->lifetimeMs = lifetimeMs;
    mole->alive = 1;
    mole->killed = 0;
    mole->startTime = GetTickCount();
    mole->messageQueue = queue;

    DWORD threadId;
    mole->thread = CreateThread(
        NULL,
        0,
        MoleThreadFunc,
        mole,
        0,
        &threadId
    );

    if (mole->thread == NULL) {
        free(mole);
        return NULL;
    }

    return mole;
}

BOOL IsMoleAlive(Mole* mole) {
    return InterlockedCompareExchange(&mole->alive, 0, 0) == 1;
}

BOOL KillMole(Mole* mole) {
    if (InterlockedCompareExchange(&mole->alive, 0, 1) == 1) {
        InterlockedExchange(&mole->killed, 1);

        if (mole->messageQueue != NULL) {
            Message msg(MSG_MOLE_KILLED, mole->id, mole->key);
            TryPushMessage(mole->messageQueue, msg);
        }

        return TRUE;
    }

    return FALSE;
}

void WaitMole(Mole* mole) {
    if (mole->thread != NULL) {
        WaitForSingleObject(mole->thread, INFINITE);
    }
}

void DestroyMole(Mole* mole) {
    if (mole == NULL) return;

    if (mole->thread != NULL) {
        CloseHandle(mole->thread);
    }

    free(mole);
}
