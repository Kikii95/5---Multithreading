#include "Message.h"
#include "WorkerThread.h"
#include "ThreadManager.h"
#include "../Entity/Ball.h"
#include <stdio.h>

// ============================================
// COLLISION MESSAGE EXECUTE
// ============================================
void CollisionMessage::Execute(WorkerThread* thread) {
    Entity* target = thread->FindEntityByID(targetEntityID);

    if (!target || target->mIsDead) {
        printf("[Thread %d] CollisionMessage: Target entity %d not found or dead\n",
               thread->GetThreadID(), targetEntityID);
        return;
    }

    target->mStats.hp -= damageAmount;

    printf("[Thread %d] CollisionMessage: Ball %d hit Ball %d (damage: %.1f, HP: %.1f -> %.1f)\n",
           thread->GetThreadID(), sourceEntityID, targetEntityID,
           damageAmount, target->mStats.hp + damageAmount, target->mStats.hp);

    if (target->mStats.hp <= 0) {
        target->mIsDead = true;

        printf("[Thread %d] *** Ball %d KILLED by Ball %d ***\n",
               thread->GetThreadID(), targetEntityID, sourceEntityID);

        printf("[Thread %d] -> Sending kill reward to Ball %d\n",
               thread->GetThreadID(), sourceEntityID);
    }
}

// ============================================
// TRANSITION MESSAGE EXECUTE
// ============================================
void TransitionMessage::Execute(WorkerThread* thread) {
    if (entityData.isDead) {
        printf("[Thread %d] TransitionMessage: Ball %d is dead, ignoring\n",
               thread->GetThreadID(), entityData.id);
        return;
    }

    Ball* newBall = new Ball(entityData.id, entityData.pos, entityData.radius);
    newBall->Deserialize(entityData);
    newBall->mThreadIndex = thread->GetThreadID();
    newBall->mIsTransitioning = false;  

    thread->AddEntity(newBall);

    printf("[Thread %d] TransitionMessage: Ball %d transferred from Thread %d (pos: %.1f, %.1f)\n",
           thread->GetThreadID(), entityData.id, fromThreadID,
           entityData.pos.x, entityData.pos.y);
}

// ============================================
// KILL REWARD MESSAGE EXECUTE
// ============================================
void KillRewardMessage::Execute(WorkerThread* thread) {
    Entity* killer = thread->FindEntityByID(killerEntityID);

    if (!killer || killer->mIsDead) {
        printf("[Thread %d] KillRewardMessage: Killer entity %d not found\n",
               thread->GetThreadID(), killerEntityID);
        return;
    }

    killer->mStats.hp += hpGain;

    Ball* killerBall = dynamic_cast<Ball*>(killer);
    if (killerBall) {
        killerBall->mKillCount++;
        printf("[Thread %d] KillRewardMessage: Ball %d gains %.1f HP (Total: %.1f, Kills: %d)\n",
               thread->GetThreadID(), killerEntityID, hpGain,
               killer->mStats.hp, killerBall->mKillCount);
    }
}
