#pragma once
#include <vector>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "SafeQueue.h"
#include "Message.h"
#include "../Entity/Entity.h"
class ThreadManager;
class WorkerThread {
private:
    int mThreadID;
    HANDLE mHandle;
    DWORD mWinThreadID;
    SafeQueue<Message*> mMessageQueue;
    std::vector<Entity*> mEntities;  
    bool mShouldExit;
    ThreadManager* mManager;  
    int mCollisionsDetected;
    int mMessagesProcessed;
public:
    WorkerThread(int id, ThreadManager* manager);
    ~WorkerThread();
    void Start();
    void Stop();
    void Join();
    void AddEntity(Entity* entity);
    void RemoveEntity(Entity* entity);
    Entity* FindEntityByID(int id);
    const std::vector<Entity*>& GetEntities() const { return mEntities; }
    void PushMessage(Message* msg);
    int GetThreadID() const { return mThreadID; }
    int GetCollisionsDetected() const { return mCollisionsDetected; }
    int GetMessagesProcessed() const { return mMessagesProcessed; }
private:
    static DWORD WINAPI ThreadProc(LPVOID param);
    void Update(float deltaTime);
    void ProcessMessages();
    void DetectCollisions();
    void CheckTransitions();
};
