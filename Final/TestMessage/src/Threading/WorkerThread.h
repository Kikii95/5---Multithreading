#pragma once
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "SafeQueue.h"
#include "Message.h"
#include "../Entity/Entity.h"

// Forward declaration
class ThreadManager;

// Thread worker (gere une zone de la map)
class WorkerThread {
private:
    int mThreadID;
    HANDLE mHandle;
    DWORD mWinThreadID;

    SafeQueue<Message*> mMessageQueue;
    std::vector<Entity*> mEntities;  // Entities possedees

    bool mShouldExit;
    ThreadManager* mManager;  // Reference au manager global

    // Stats
    int mCollisionsDetected;
    int mMessagesProcessed;

public:
    WorkerThread(int id, ThreadManager* manager);
    ~WorkerThread();

    // Demarrage/Arret
    void Start();
    void Stop();
    void Join();

    // Gestion entities
    void AddEntity(Entity* entity);
    void RemoveEntity(Entity* entity);
    Entity* FindEntityByID(int id);
    const std::vector<Entity*>& GetEntities() const { return mEntities; }

    // Messages
    void PushMessage(Message* msg);

    // Accesseurs
    int GetThreadID() const { return mThreadID; }
    int GetCollisionsDetected() const { return mCollisionsDetected; }
    int GetMessagesProcessed() const { return mMessagesProcessed; }

private:
    // Thread loop (static pour WIN API)
    static DWORD WINAPI ThreadProc(LPVOID param);

    // Methodes internes
    void Update(float deltaTime);
    void ProcessMessages();
    void DetectCollisions();
    void CheckTransitions();
};
