#include "WorkerThread.h"
#include "ThreadManager.h"
#include "../Entity/Player.h"
#include "../Entity/Bonus.h"
#include <stdio.h>
#include <algorithm>
#include <chrono>
#include <thread>
WorkerThread::WorkerThread(int id, ThreadManager* manager)
    : mThreadID(id), mHandle(NULL), mWinThreadID(0),
      mShouldExit(false), mManager(manager),
      mCollisionsDetected(0), mMessagesProcessed(0) {
}
WorkerThread::~WorkerThread() {
    Stop();
    for (Entity* e : mEntities) {
        delete e;
    }
    mEntities.clear();
}
void WorkerThread::Start() {
    mShouldExit = false;
    mHandle = CreateThread(
        NULL,
        0,
        ThreadProc,
        this,
        0,
        &mWinThreadID
    );
    if (mHandle == NULL) {
        printf("[ERROR] Thread %d creation failed!\n", mThreadID);
    } else {
        printf("[Thread %d] Started (Win Thread ID: %lu)\n", mThreadID, mWinThreadID);
    }
}
void WorkerThread::Stop() {
    mShouldExit = true;
}
void WorkerThread::Join() {
    if (mHandle != NULL) {
        WaitForSingleObject(mHandle, INFINITE);
        CloseHandle(mHandle);
        mHandle = NULL;
        printf("[Thread %d] Joined\n", mThreadID);
    }
}
void WorkerThread::AddEntity(Entity* entity) {
    entity->mThreadIndex = mThreadID;
    mEntities.push_back(entity);
}
void WorkerThread::RemoveEntity(Entity* entity) {
    auto it = std::find(mEntities.begin(), mEntities.end(), entity);
    if (it != mEntities.end()) {
        mEntities.erase(it);
    }
}
Entity* WorkerThread::FindEntityByID(int id) {
    for (Entity* e : mEntities) {
        if (e->mID == id) return e;
    }
    return nullptr;
}
void WorkerThread::PushMessage(Message* msg) {
    mMessageQueue.Push(msg);
}
DWORD WINAPI WorkerThread::ThreadProc(LPVOID param) {
    WorkerThread* thread = (WorkerThread*)param;
    const float DELTA_TIME = 0.016f;  
    const int SLEEP_MS = 16;          
    int frameCount = 0;
    while (!thread->mShouldExit) {
        thread->ProcessMessages();
        thread->Update(DELTA_TIME);
        thread->DetectCollisions();
        thread->CheckTransitions();
        Sleep(SLEEP_MS);
        frameCount++;
        if (frameCount % 300 == 0) {
            printf("[Thread %d] Alive: %zu entities, Collisions: %d, Messages: %d\n",
                   thread->mThreadID, thread->mEntities.size(),
                   thread->mCollisionsDetected, thread->mMessagesProcessed);
        }
    }
    printf("[Thread %d] Exiting\n", thread->mThreadID);
    return 0;
}
void WorkerThread::ProcessMessages() {
    Message* msg;
    int count = 0;
    while (mMessageQueue.Pop(msg)) {
        printf("[Thread %d] <- Received %s message\n", mThreadID, msg->GetTypeName());
        msg->Execute(this);
        delete msg;
        count++;
        mMessagesProcessed++;
    }
}
void WorkerThread::Update(float deltaTime) {
    for (Entity* entity : mEntities) {
        if (!entity->mIsDead && !entity->mIsTransitioning) {
            entity->Update(deltaTime);
        }
    }
}
void WorkerThread::DetectCollisions() {
    std::vector<Entity*> allEntities = mManager->GetAllEntities();
    for (Entity* myEntity : mEntities) {
        if (myEntity->mIsDead || myEntity->mIsTransitioning) continue;
        Bonus* myBonus = dynamic_cast<Bonus*>(myEntity);
        if (myBonus) continue;
        for (Entity* otherEntity : allEntities) {
            if (otherEntity == myEntity) continue;
            if (otherEntity->mIsDead || otherEntity->mIsTransitioning) continue;
            if (myEntity->Collide(otherEntity)) {
                Player* myPlayer = dynamic_cast<Player*>(myEntity);
                Bonus* otherBonus = dynamic_cast<Bonus*>(otherEntity);
                if (myPlayer && otherBonus && !otherBonus->mIsPickedUp) {
                    myPlayer->ApplyBonus((int)otherBonus->mType);
                    otherBonus->PickUp();  
                    printf("[Thread %d] *** Player %d picked up Bonus %d ***\n",
                           mThreadID, myPlayer->mID, otherBonus->mID);
                    continue;  
                }
                Player* otherPlayer = dynamic_cast<Player*>(otherEntity);
                if (myPlayer && otherPlayer) {
                    if (otherEntity->mThreadIndex == mThreadID) {
                        float damage1 = myPlayer->CalculateDamage(otherPlayer);
                        float damage2 = otherPlayer->CalculateDamage(myPlayer);
                        myPlayer->TakeDamage(damage2);
                        otherPlayer->TakeDamage(damage1);
                        myPlayer->BounceOff(otherPlayer);
                        printf("[Thread %d] LOCAL collision: Player %d (HP %.1f) vs Player %d (HP %.1f)\n",
                               mThreadID, myPlayer->mID, myPlayer->mStats.hp,
                               otherPlayer->mID, otherPlayer->mStats.hp);
                        mCollisionsDetected++;
                        if (myPlayer->mIsDead) {
                            otherPlayer->mKillCount++;
                            otherPlayer->mStats.hp += 10.0f;
                            printf("[Thread %d] *** Player %d KILLED by Player %d (local) ***\n",
                                   mThreadID, myPlayer->mID, otherPlayer->mID);
                        }
                        if (otherPlayer->mIsDead) {
                            myPlayer->mKillCount++;
                            myPlayer->mStats.hp += 10.0f;
                            printf("[Thread %d] *** Player %d KILLED by Player %d (local) ***\n",
                                   mThreadID, otherPlayer->mID, myPlayer->mID);
                        }
                    }
                    else if (mThreadID < otherEntity->mThreadIndex) {
                        float damageToOther = myPlayer->CalculateDamage(otherPlayer);
                        float damageToMe = otherPlayer->CalculateDamage(myPlayer);
                        myPlayer->TakeDamage(damageToMe);
                        myPlayer->BounceOff(otherPlayer);
                        printf("[Thread %d] REMOTE collision: Player %d (HP %.1f) vs Player %d (Thread %d)\n",
                               mThreadID, myPlayer->mID, myPlayer->mStats.hp,
                               otherPlayer->mID, otherPlayer->mThreadIndex);
                        mCollisionsDetected++;
                        CollisionMessage* msg = new CollisionMessage(
                            myPlayer->mID,
                            otherPlayer->mID,
                            damageToOther,
                            (myPlayer->mPos + otherPlayer->mPos) * 0.5f
                        );
                        WorkerThread* otherThread = mManager->GetThread(otherPlayer->mThreadIndex);
                        otherThread->PushMessage(msg);
                        printf("[Thread %d] -> Sent COLLISION message to Thread %d\n",
                               mThreadID, otherPlayer->mThreadIndex);
                        if (myPlayer->mIsDead) {
                            printf("[Thread %d] *** Player %d KILLED (remote collision) ***\n",
                                   mThreadID, myPlayer->mID);
                            KillRewardMessage* reward = new KillRewardMessage(
                                otherPlayer->mID, 10.0f, myPlayer->mID
                            );
                            otherThread->PushMessage(reward);
                        }
                    }
                }
            }
        }
    }
}
void WorkerThread::CheckTransitions() {
    std::vector<Entity*> entitiesToTransfer;
    for (Entity* entity : mEntities) {
        if (entity->mIsDead) continue;
        Bonus* bonus = dynamic_cast<Bonus*>(entity);
        if (bonus) continue;
        int expectedThreadID = mManager->CalculateThreadFromPosition(entity->mPos);
        if (expectedThreadID != mThreadID) {
            entitiesToTransfer.push_back(entity);
        }
    }
    for (Entity* entity : entitiesToTransfer) {
        int targetThreadID = mManager->CalculateThreadFromPosition(entity->mPos);
        printf("[Thread %d] Player %d transitioning to Thread %d (pos: %.1f, %.1f)\n",
               mThreadID, entity->mID, targetThreadID, entity->mPos.x, entity->mPos.y);
        entity->mIsTransitioning = true;
        TransitionMessage* msg = new TransitionMessage(
            entity->Serialize(),
            mThreadID,
            targetThreadID
        );
        WorkerThread* targetThread = mManager->GetThread(targetThreadID);
        targetThread->PushMessage(msg);
        printf("[Thread %d] -> Sent TRANSITION message to Thread %d\n",
               mThreadID, targetThreadID);
        RemoveEntity(entity);
        delete entity;
    }
}
