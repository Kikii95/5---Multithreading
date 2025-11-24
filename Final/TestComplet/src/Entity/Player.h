#pragma once
#include "Entity.h"
#include "../Core/Utils.h"
class Player : public Entity {
public:
    int mKillCount;      
    int mBonusCount;     
    bool mHasShield;     
    float mSpeedMultiplier;   
    float mDamageMultiplier;  
    Player(int id, Vector2 pos, float radius)
        : Entity(id, pos, radius), mKillCount(0), mBonusCount(0),
          mHasShield(false), mSpeedMultiplier(1.0f), mDamageMultiplier(1.0f) {
        mStats.hp = Utils::RandomFloat(80.0f, 120.0f);
        mStats.speed = Utils::RandomFloat(60.0f, 100.0f);
        mStats.damage = Utils::RandomFloat(10.0f, 20.0f);
        mDirection = Utils::RandomDirection();
        mRadius = 10.0f + (mStats.hp / 10.0f);
    }
    void Update(float deltaTime) override {
        if (mIsDead || mIsTransitioning) return;
        float sizeRatio = mStats.hp / 100.0f;
        float baseSpeed = 80.0f / sqrtf(sizeRatio);
        mStats.speed = baseSpeed * mSpeedMultiplier;  
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
        mRadius = 10.0f + (mStats.hp / 10.0f);
    }
    void OnCollide(Entity* other) override {
    }
    float CalculateDamage(Entity* other) const {
        float damageMultiplier = 1.0f;
        if (mStats.hp > other->mStats.hp) {
            damageMultiplier = 1.25f;
        }
        return mStats.damage * damageMultiplier * mDamageMultiplier;
    }
    void TakeDamage(float damage) {
        if (mHasShield) {
            mHasShield = false;
            printf("[Player %d] Shield absorbed hit!\n", mID);
            return;
        }
        mStats.hp -= damage;
        if (mStats.hp <= 0) {
            mStats.hp = 0;
            mIsDead = true;
        }
    }
    void ApplyBonus(int bonusType) {
        mBonusCount++;
        switch (bonusType) {
            case 0: 
                mStats.hp += 20.0f;
                printf("[Player %d] HP Boost! HP: %.0f\n", mID, mStats.hp);
                break;
            case 1: 
                mSpeedMultiplier += 0.2f;  
                printf("[Player %d] Speed Boost! Multiplier: %.2fx\n", mID, mSpeedMultiplier);
                break;
            case 2: 
                mHasShield = true;
                printf("[Player %d] Shield activated!\n", mID);
                break;
            case 3: 
                mDamageMultiplier += 0.5f;  
                printf("[Player %d] Damage Boost! Multiplier: %.2fx\n", mID, mDamageMultiplier);
                break;
        }
    }
    void BounceOff(Player* other) {
        Vector2 collisionNormal = (other->mPos - mPos).Normalized();
        mDirection = mDirection.Reflect(collisionNormal);
        float dist = (mPos - other->mPos).Length();
        float overlap = (mRadius + other->mRadius) - dist;
        if (overlap > 0) {
            mPos = mPos - collisionNormal * (overlap / 2.0f);
        }
    }
};
