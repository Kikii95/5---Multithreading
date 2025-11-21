#include "WorkerThread.h"
#include "ThreadManager.h"
#include "../Entity/Ball.h"
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

    // Liberer entities
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

// ============================================
// THREAD LOOP (static)
// ============================================
DWORD WINAPI WorkerThread::ThreadProc(LPVOID param) {
    WorkerThread* thread = (WorkerThread*)param;

    const float DELTA_TIME = 0.016f;  // ~60 FPS
    const int SLEEP_MS = 16;          // 16ms = ~60 FPS

    int frameCount = 0;

    while (!thread->mShouldExit) {
        // 1. Traiter messages entrants
        thread->ProcessMessages();

        // 2. Update entities locales
        thread->Update(DELTA_TIME);

        // 3. Detecter collisions
        thread->DetectCollisions();

        // 4. Verifier transitions
        thread->CheckTransitions();

        // 5. Pause (simuler frame rate)
        Sleep(SLEEP_MS);

        frameCount++;

        // Log toutes les 5 secondes (~300 frames)
        if (frameCount % 300 == 0) {
            printf("[Thread %d] Alive: %zu balls, Collisions: %d, Messages: %d\n",
                   thread->mThreadID, thread->mEntities.size(),
                   thread->mCollisionsDetected, thread->mMessagesProcessed);
        }
    }

    printf("[Thread %d] Exiting\n", thread->mThreadID);
    return 0;
}

// ============================================
// PROCESS MESSAGES
// ============================================
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

// ============================================
// UPDATE ENTITIES
// ============================================
void WorkerThread::Update(float deltaTime) {
    for (Entity* entity : mEntities) {
        if (!entity->mIsDead && !entity->mIsTransitioning) {
            entity->Update(deltaTime);
        }
    }
}

// ============================================
// DETECT COLLISIONS
// ============================================
void WorkerThread::DetectCollisions() {
    // Recuperer TOUTES les entities (via ThreadManager)
    std::vector<Entity*> allEntities = mManager->GetAllEntities();

    for (Entity* myEntity : mEntities) {
        if (myEntity->mIsDead || myEntity->mIsTransitioning) continue;

        for (Entity* otherEntity : allEntities) {
            if (otherEntity == myEntity) continue;
            if (otherEntity->mIsDead || otherEntity->mIsTransitioning) continue;

            // Collision detectee ?
            if (myEntity->Collide(otherEntity)) {

                if (otherEntity->mThreadIndex == mThreadID) {
                    // ========================================
                    // COLLISION LOCALE (meme thread)
                    // ========================================
                    Ball* myBall = dynamic_cast<Ball*>(myEntity);
                    Ball* otherBall = dynamic_cast<Ball*>(otherEntity);

                    if (myBall && otherBall) {
                        // Calculer degats mutuels
                        float damage1 = myBall->CalculateDamage(otherBall);
                        float damage2 = otherBall->CalculateDamage(myBall);

                        // Appliquer sur les DEUX
                        myBall->mStats.hp -= damage2;
                        otherBall->mStats.hp -= damage1;

                        // Rebond elastique
                        myBall->BounceOff(otherBall);

                        printf("[Thread %d] LOCAL collision: Ball %d (HP %.1f) vs Ball %d (HP %.1f)\n",
                               mThreadID, myBall->mID, myBall->mStats.hp,
                               otherBall->mID, otherBall->mStats.hp);

                        mCollisionsDetected++;

                        // Gerer morts
                        if (myBall->mStats.hp <= 0) {
                            myBall->mIsDead = true;
                            otherBall->mKillCount++;
                            otherBall->mStats.hp += 10.0f;
                            printf("[Thread %d] *** Ball %d KILLED by Ball %d (local) ***\n",
                                   mThreadID, myBall->mID, otherBall->mID);
                        }
                        if (otherBall->mStats.hp <= 0) {
                            otherBall->mIsDead = true;
                            myBall->mKillCount++;
                            myBall->mStats.hp += 10.0f;
                            printf("[Thread %d] *** Ball %d KILLED by Ball %d (local) ***\n",
                                   mThreadID, otherBall->mID, myBall->mID);
                        }
                    }
                }
                else if (mThreadID < otherEntity->mThreadIndex) {
                    // ========================================
                    // COLLISION REMOTE (thread different)
                    // Thread avec ID plus petit gere
                    // ========================================
                    Ball* myBall = dynamic_cast<Ball*>(myEntity);
                    Ball* otherBall = dynamic_cast<Ball*>(otherEntity);

                    if (myBall && otherBall) {
                        float damageToOther = myBall->CalculateDamage(otherBall);
                        float damageToMe = otherBall->CalculateDamage(myBall);

                        // Appliquer degats sur MOI localement
                        myBall->mStats.hp -= damageToMe;

                        // Rebond sur moi seulement (l'autre thread gerera son rebond)
                        myBall->BounceOff(otherBall);

                        printf("[Thread %d] REMOTE collision: Ball %d (HP %.1f) vs Ball %d (Thread %d)\n",
                               mThreadID, myBall->mID, myBall->mStats.hp,
                               otherBall->mID, otherBall->mThreadIndex);

                        mCollisionsDetected++;

                        // Envoyer message a l'autre thread
                        CollisionMessage* msg = new CollisionMessage(
                            myBall->mID,
                            otherBall->mID,
                            damageToOther,
                            (myBall->mPos + otherBall->mPos) * 0.5f
                        );

                        WorkerThread* otherThread = mManager->GetThread(otherBall->mThreadIndex);
                        otherThread->PushMessage(msg);

                        printf("[Thread %d] -> Sent COLLISION message to Thread %d\n",
                               mThreadID, otherBall->mThreadIndex);

                        // Gerer MA propre mort
                        if (myBall->mStats.hp <= 0) {
                            myBall->mIsDead = true;
                            printf("[Thread %d] *** Ball %d KILLED (remote collision) ***\n",
                                   mThreadID, myBall->mID);

                            // Envoyer kill reward a l'autre
                            KillRewardMessage* reward = new KillRewardMessage(
                                otherBall->mID, 10.0f, myBall->mID
                            );
                            otherThread->PushMessage(reward);
                        }
                    }
                }
                // Sinon : l'autre thread gere (mThreadID > otherEntity->mThreadIndex)
            }
        }
    }
}

// ============================================
// CHECK TRANSITIONS
// ============================================
void WorkerThread::CheckTransitions() {
    std::vector<Entity*> entitiesToTransfer;

    for (Entity* entity : mEntities) {
        if (entity->mIsDead) continue;

        int expectedThreadID = mManager->CalculateThreadFromPosition(entity->mPos);

        if (expectedThreadID != mThreadID) {
            entitiesToTransfer.push_back(entity);
        }
    }

    for (Entity* entity : entitiesToTransfer) {
        int targetThreadID = mManager->CalculateThreadFromPosition(entity->mPos);

        printf("[Thread %d] Ball %d transitioning to Thread %d (pos: %.1f, %.1f)\n",
               mThreadID, entity->mID, targetThreadID, entity->mPos.x, entity->mPos.y);

        // Marquer en transition (immunite)
        entity->mIsTransitioning = true;

        // Creer message
        TransitionMessage* msg = new TransitionMessage(
            entity->Serialize(),
            mThreadID,
            targetThreadID
        );

        // Envoyer au thread cible
        WorkerThread* targetThread = mManager->GetThread(targetThreadID);
        targetThread->PushMessage(msg);

        printf("[Thread %d] -> Sent TRANSITION message to Thread %d\n",
               mThreadID, targetThreadID);

        // Retirer de MA liste
        RemoveEntity(entity);
        delete entity;
    }
}
