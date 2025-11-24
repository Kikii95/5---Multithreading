#pragma once
#include "../Core/Vector2.h"
#include "../Core/Utils.h"

struct Stats {
    float hp;
    float speed;
    float damage;

    Stats() : hp(100.0f), speed(80.0f), damage(15.0f) {}
    Stats(float _hp, float _speed, float _damage)
        : hp(_hp), speed(_speed), damage(_damage) {}
};

class Entity {
public:
    int mID;                    
    Vector2 mPos;             
    Vector2 mDirection;         
    float mRadius;              
    Stats mStats;               

    int mThreadIndex;           
    bool mIsDead;              
    bool mIsTransitioning;      

    Entity(int id, Vector2 pos, float radius)
        : mID(id), mPos(pos), mRadius(radius),
          mThreadIndex(-1), mIsDead(false), mIsTransitioning(false) {
        mDirection = Vector2(0, 0);
    }

    virtual ~Entity() {}

    virtual void Update(float deltaTime) = 0;
    virtual void OnCollide(Entity* other) = 0;

    bool Collide(Entity* other) const {
        if (!other || other == this) return false;
        if (mIsDead || other->mIsDead) return false;
        if (mIsTransitioning || other->mIsTransitioning) return false;

        float dist = (mPos - other->mPos).Length();
        return dist < (mRadius + other->mRadius);
    }

    struct SerializedData {
        int id;
        Vector2 pos;
        Vector2 direction;
        float radius;
        Stats stats;
        bool isDead;
        int killCount;
    };

    SerializedData Serialize() const;
    void Deserialize(const SerializedData& data);
};
