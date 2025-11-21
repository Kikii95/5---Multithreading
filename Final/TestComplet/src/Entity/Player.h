#pragma once
#include "Entity.h"
#include "../Core/Utils.h"

// Player = entité mobile avec système powerups
class Player : public Entity {
public:
    int mKillCount;      // Nombre de kills
    int mBonusCount;     // Nombre de bonus ramassés

    // Powerups permanents
    bool mHasShield;     // Shield actif (absorbe 1 coup)
    float mSpeedMultiplier;   // Multiplicateur speed (1.0 base, +0.2 par boost)
    float mDamageMultiplier;  // Multiplicateur damage (1.0 base, +0.5 par boost)

    Player(int id, Vector2 pos, float radius)
        : Entity(id, pos, radius), mKillCount(0), mBonusCount(0),
          mHasShield(false), mSpeedMultiplier(1.0f), mDamageMultiplier(1.0f) {
        // Stats aléatoires de base
        mStats.hp = Utils::RandomFloat(80.0f, 120.0f);
        mStats.speed = Utils::RandomFloat(60.0f, 100.0f);
        mStats.damage = Utils::RandomFloat(10.0f, 20.0f);

        // Direction aléatoire
        mDirection = Utils::RandomDirection();

        // Radius basé sur HP
        mRadius = 10.0f + (mStats.hp / 10.0f);
    }

    void Update(float deltaTime) override {
        if (mIsDead || mIsTransitioning) return;

        // Vitesse ajustée selon taille (plus gros = plus lent) ET powerups
        float sizeRatio = mStats.hp / 100.0f;
        float baseSpeed = 80.0f / sqrtf(sizeRatio);
        mStats.speed = baseSpeed * mSpeedMultiplier;  // Appliquer boost speed

        // Mouvement
        mPos = mPos + mDirection * mStats.speed * deltaTime;

        // Rebond sur murs
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

        // Ajuster radius selon HP
        mRadius = 10.0f + (mStats.hp / 10.0f);
    }

    void OnCollide(Entity* other) override {
        // Géré dans WorkerThread
    }

    float CalculateDamage(Entity* other) const {
        // Dégâts = base damage * 1.25 si plus gros * powerup damage
        float damageMultiplier = 1.0f;
        if (mStats.hp > other->mStats.hp) {
            damageMultiplier = 1.25f;
        }
        return mStats.damage * damageMultiplier * mDamageMultiplier;
    }

    // Prendre des dégâts (avec shield)
    void TakeDamage(float damage) {
        if (mHasShield) {
            // Shield absorbe le coup
            mHasShield = false;
            printf("[Player %d] Shield absorbed hit!\n", mID);
            return;
        }

        // Dégâts normaux
        mStats.hp -= damage;
        if (mStats.hp <= 0) {
            mStats.hp = 0;
            mIsDead = true;
        }
    }

    // Appliquer bonus (pickup)
    void ApplyBonus(int bonusType) {
        mBonusCount++;

        switch (bonusType) {
            case 0: // HP_BOOST
                mStats.hp += 20.0f;
                printf("[Player %d] HP Boost! HP: %.0f\n", mID, mStats.hp);
                break;

            case 1: // SPEED_BOOST
                mSpeedMultiplier += 0.2f;  // +20% permanent
                printf("[Player %d] Speed Boost! Multiplier: %.2fx\n", mID, mSpeedMultiplier);
                break;

            case 2: // SHIELD
                mHasShield = true;
                printf("[Player %d] Shield activated!\n", mID);
                break;

            case 3: // DAMAGE_BOOST
                mDamageMultiplier += 0.5f;  // +50% permanent
                printf("[Player %d] Damage Boost! Multiplier: %.2fx\n", mID, mDamageMultiplier);
                break;
        }
    }

    // Rebond élastique sur autre player
    void BounceOff(Player* other) {
        // Direction de collision
        Vector2 collisionNormal = (other->mPos - mPos).Normalized();

        // Refléchir direction
        mDirection = mDirection.Reflect(collisionNormal);

        // Séparer les players (éviter overlap)
        float dist = (mPos - other->mPos).Length();
        float overlap = (mRadius + other->mRadius) - dist;
        if (overlap > 0) {
            mPos = mPos - collisionNormal * (overlap / 2.0f);
        }
    }
};
