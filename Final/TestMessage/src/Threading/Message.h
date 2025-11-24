#pragma once
#include "../Entity/Entity.h"

class WorkerThread;

enum class MessageType {
    COLLISION,
    TRANSITION,
    KILL_REWARD
};

// ============================================
// MESSAGE BASE 
// ============================================
class Message {
public:
    MessageType mType;

    Message(MessageType type) : mType(type) {}
    virtual ~Message() {}

    virtual void Execute(WorkerThread* thread) = 0;

    virtual const char* GetTypeName() const = 0;
};

// ============================================
// COLLISION MESSAGE
// ============================================
class CollisionMessage : public Message {
public:
    int sourceEntityID;   
    int targetEntityID;   
    float damageAmount;   
    Vector2 collisionPoint;

    CollisionMessage(int source, int target, float damage, Vector2 point)
        : Message(MessageType::COLLISION),
          sourceEntityID(source), targetEntityID(target),
          damageAmount(damage), collisionPoint(point) {}

    void Execute(WorkerThread* thread) override;
    const char* GetTypeName() const override { return "COLLISION"; }
};

// ============================================
// TRANSITION MESSAGE
// ============================================
class TransitionMessage : public Message {
public:
    Entity::SerializedData entityData;
    int fromThreadID;
    int toThreadID;

    TransitionMessage(const Entity::SerializedData& data, int from, int to)
        : Message(MessageType::TRANSITION),
          entityData(data), fromThreadID(from), toThreadID(to) {}

    void Execute(WorkerThread* thread) override;
    const char* GetTypeName() const override { return "TRANSITION"; }
};

// ============================================
// KILL REWARD MESSAGE
// ============================================
class KillRewardMessage : public Message {
public:
    int killerEntityID;
    float hpGain;
    int victimID;  

    KillRewardMessage(int killer, float gain, int victim)
        : Message(MessageType::KILL_REWARD),
          killerEntityID(killer), hpGain(gain), victimID(victim) {}

    void Execute(WorkerThread* thread) override;
    const char* GetTypeName() const override { return "KILL_REWARD"; }
};
