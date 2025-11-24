#include "Message.h"
#include "WorkerThread.h"
#include "ThreadManager.h"
#include "../Entity/Player.h"
#include <stdio.h>
void CollisionMessage::Execute(WorkerThread* thread) {
    Entity* target = thread->FindEntityByID(targetEntityID);
    if (!target || target->mIsDead) {
        printf("[Thread %d] CollisionMessage: Target entity %d not found or dead\n",
               thread->GetThreadID(), targetEntityID);
        return;
    }
    Player* targetPlayer = dynamic_cast<Player*>(target);
    if (!targetPlayer) return;  
    float hpBefore = targetPlayer->mStats.hp;
    targetPlayer->TakeDamage(damageAmount);
    printf("[Thread %d] CollisionMessage: Player %d hit Player %d (damage: %.1f, HP: %.1f -> %.1f)\n",
           thread->GetThreadID(), sourceEntityID, targetEntityID,
           damageAmount, hpBefore, targetPlayer->mStats.hp);
    if (targetPlayer->mIsDead) {
        printf("[Thread %d] *** Player %d KILLED by Player %d ***\n",
               thread->GetThreadID(), targetEntityID, sourceEntityID);
        printf("[Thread %d] -> Sending kill reward to Player %d\n",
               thread->GetThreadID(), sourceEntityID);
    }
}
void TransitionMessage::Execute(WorkerThread* thread) {
    if (entityData.isDead) {
        printf("[Thread %d] TransitionMessage: Player %d is dead, ignoring\n",
               thread->GetThreadID(), entityData.id);
        return;
    }
    Player* newPlayer = new Player(entityData.id, entityData.pos, entityData.radius);
    newPlayer->Deserialize(entityData);
    newPlayer->mThreadIndex = thread->GetThreadID();
    newPlayer->mIsTransitioning = false;  
    thread->AddEntity(newPlayer);
    printf("[Thread %d] TransitionMessage: Player %d transferred from Thread %d (pos: %.1f, %.1f)\n",
           thread->GetThreadID(), entityData.id, fromThreadID,
           entityData.pos.x, entityData.pos.y);
}
void KillRewardMessage::Execute(WorkerThread* thread) {
    Entity* killer = thread->FindEntityByID(killerEntityID);
    if (!killer || killer->mIsDead) {
        printf("[Thread %d] KillRewardMessage: Killer entity %d not found\n",
               thread->GetThreadID(), killerEntityID);
        return;
    }
    killer->mStats.hp += hpGain;
    Player* killerPlayer = dynamic_cast<Player*>(killer);
    if (killerPlayer) {
        killerPlayer->mKillCount++;
        printf("[Thread %d] KillRewardMessage: Player %d gains %.1f HP (Total: %.1f, Kills: %d)\n",
               thread->GetThreadID(), killerEntityID, hpGain,
               killer->mStats.hp, killerPlayer->mKillCount);
    }
}
