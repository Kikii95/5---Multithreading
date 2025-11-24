#pragma once
#include "Entity.h"
#include "../Core/Utils.h"

class Ball : public Entity {
public:
    int mKillCount; 

    Ball(int id, Vector2 pos, float radius)
        : Entity(id, pos, radius), mKillCount(0) {
        mStats.hp = Utils::RandomFloat(80.0f, 120.0f);
        mStats.speed = Utils::RandomFloat(60.0f, 100.0f);
        mStats.damage = Utils::RandomFloat(10.0f, 20.0f);

        mDirection = Utils::RandomDirection();

        mRadius = 10.0f + (mStats.hp / 10.0f);
    }

    void Update(float deltaTime) override {
        if (mIsDead || mIsTransitioning) return;

        mPos = mPos + mDirection * mStats.speed * deltaTime;

        bool bounced = false;

        if (mPos.x - mRadius < 0) {
            mPos.x = mRadius;
            mDirection = mDirection.Reflect(Vector2(1, 0));
            bounced = true;
        }
        else if (mPos.x + mRadius > Config::MAP_WIDTH) {
            mPos.x = Config::MAP_WIDTH - mRadius;
            mDirection = mDirection.Reflect(Vector2(-1, 0));  
            bounced = true;
        }

        if (mPos.y - mRadius < 0) {
            mPos.y = mRadius;
            mDirection = mDirection.Reflect(Vector2(0, 1));  
            bounced = true;
        }
        else if (mPos.y + mRadius > Config::MAP_HEIGHT) {
            mPos.y = Config::MAP_HEIGHT - mRadius;
            mDirection = mDirection.Reflect(Vector2(0, -1)); 
            bounced = true;
        }

        float sizeRatio = mStats.hp / 100.0f;
        mStats.speed = 80.0f / sqrtf(sizeRatio);

        mRadius = 10.0f + (mStats.hp / 10.0f);
    }

    void OnCollide(Entity* other) override {
    }

    float CalculateDamage(Entity* other) const {
        float damageMultiplier = 1.0f;
        if (mStats.hp > other->mStats.hp) {
            damageMultiplier = 1.25f;
        }
        return mStats.damage * damageMultiplier;
    }

    void BounceOff(Ball* other) {
        Vector2 collisionNormal = (other->mPos - mPos).Normalized();

        mDirection = mDirection.Reflect(collisionNormal);

        float dist = (mPos - other->mPos).Length();
        float overlap = (mRadius + other->mRadius) - dist;
        if (overlap > 0) {
            mPos = mPos - collisionNormal * (overlap / 2.0f);
        }
    }
};
