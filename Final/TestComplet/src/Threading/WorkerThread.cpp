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
            printf("[Thread %d] Alive: %zu entities, Collisions: %d, Messages: %d\n",
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

        // Si mon entite est un Bonus, skip (bonus ne collide pas activement)
        Bonus* myBonus = dynamic_cast<Bonus*>(myEntity);
        if (myBonus) continue;

        for (Entity* otherEntity : allEntities) {
            if (otherEntity == myEntity) continue;
            if (otherEntity->mIsDead || otherEntity->mIsTransitioning) continue;

            // Collision detectee ?
            if (myEntity->Collide(otherEntity)) {

                // ========================================
                // CAS 1 : Player vs Bonus (PICKUP)
                // ========================================
                Player* myPlayer = dynamic_cast<Player*>(myEntity);
                Bonus* otherBonus = dynamic_cast<Bonus*>(otherEntity);

                if (myPlayer && otherBonus && !otherBonus->mIsPickedUp) {
                    // Pickup !
                    myPlayer->ApplyBonus((int)otherBonus->mType);
                    otherBonus->PickUp();  // Marque comme mort

                    printf("[Thread %d] *** Player %d picked up Bonus %d ***\n",
                           mThreadID, myPlayer->mID, otherBonus->mID);

                    continue;  // Pas de collision physique avec bonus
                }

                // ========================================
                // CAS 2 : Player vs Player
                // ========================================
                Player* otherPlayer = dynamic_cast<Player*>(otherEntity);

                if (myPlayer && otherPlayer) {
                    if (otherEntity->mThreadIndex == mThreadID) {
                        // ========================================
                        // COLLISION LOCALE (meme thread)
                        // ========================================
                        // Calculer degats mutuels
                        float damage1 = myPlayer->CalculateDamage(otherPlayer);
                        float damage2 = otherPlayer->CalculateDamage(myPlayer);

                        // Appliquer degats (avec shield)
                        myPlayer->TakeDamage(damage2);
                        otherPlayer->TakeDamage(damage1);

                        // Rebond elastique
                        myPlayer->BounceOff(otherPlayer);

                        printf("[Thread %d] LOCAL collision: Player %d (HP %.1f) vs Player %d (HP %.1f)\n",
                               mThreadID, myPlayer->mID, myPlayer->mStats.hp,
                               otherPlayer->mID, otherPlayer->mStats.hp);

                        mCollisionsDetected++;

                        // Gerer morts
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
                        // ========================================
                        // COLLISION REMOTE (thread different)
                        // Thread avec ID plus petit gere
                        // ========================================
                        float damageToOther = myPlayer->CalculateDamage(otherPlayer);
                        float damageToMe = otherPlayer->CalculateDamage(myPlayer);

                        // Appliquer degats sur MOI localement (avec shield)
                        myPlayer->TakeDamage(damageToMe);

                        // Rebond sur moi seulement (l'autre thread gerera son rebond)
                        myPlayer->BounceOff(otherPlayer);

                        printf("[Thread %d] REMOTE collision: Player %d (HP %.1f) vs Player %d (Thread %d)\n",
                               mThreadID, myPlayer->mID, myPlayer->mStats.hp,
                               otherPlayer->mID, otherPlayer->mThreadIndex);

                        mCollisionsDetected++;

                        // Envoyer message a l'autre thread
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

                        // Gerer MA propre mort
                        if (myPlayer->mIsDead) {
                            printf("[Thread %d] *** Player %d KILLED (remote collision) ***\n",
                                   mThreadID, myPlayer->mID);

                            // Envoyer kill reward a l'autre
                            KillRewardMessage* reward = new KillRewardMessage(
                                otherPlayer->mID, 10.0f, myPlayer->mID
                            );
                            otherThread->PushMessage(reward);
                        }
                    }
                    // Sinon : l'autre thread gere (mThreadID > otherEntity->mThreadIndex)
                }
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

        // Bonus ne transitionnent pas (statiques)
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
