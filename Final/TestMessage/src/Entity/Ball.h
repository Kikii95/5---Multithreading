#pragma once
#include "Entity.h"
#include "../Core/Utils.h"

// Ball = boule rebondissante
class Ball : public Entity {
public:
    int mKillCount;  // Nombre de kills

    Ball(int id, Vector2 pos, float radius)
        : Entity(id, pos, radius), mKillCount(0) {
        // Stats aleatoires
        mStats.hp = Utils::RandomFloat(80.0f, 120.0f);
        mStats.speed = Utils::RandomFloat(60.0f, 100.0f);
        mStats.damage = Utils::RandomFloat(10.0f, 20.0f);

        // Direction aleatoire
        mDirection = Utils::RandomDirection();

        // Radius base sur HP
        mRadius = 10.0f + (mStats.hp / 10.0f);
    }

    void Update(float deltaTime) override {
        if (mIsDead || mIsTransitioning) return;

        // Mouvement
        mPos = mPos + mDirection * mStats.speed * deltaTime;

        // Rebond sur murs
        bool bounced = false;

        if (mPos.x - mRadius < 0) {
            mPos.x = mRadius;
            mDirection = mDirection.Reflect(Vector2(1, 0));  // Normale droite
            bounced = true;
        }
        else if (mPos.x + mRadius > Config::MAP_WIDTH) {
            mPos.x = Config::MAP_WIDTH - mRadius;
            mDirection = mDirection.Reflect(Vector2(-1, 0));  // Normale gauche
            bounced = true;
        }

        if (mPos.y - mRadius < 0) {
            mPos.y = mRadius;
            mDirection = mDirection.Reflect(Vector2(0, 1));  // Normale bas
            bounced = true;
        }
        else if (mPos.y + mRadius > Config::MAP_HEIGHT) {
            mPos.y = Config::MAP_HEIGHT - mRadius;
            mDirection = mDirection.Reflect(Vector2(0, -1));  // Normale haut
            bounced = true;
        }

        // Ajuster vitesse selon taille (plus gros = plus lent)
        float sizeRatio = mStats.hp / 100.0f;
        mStats.speed = 80.0f / sqrtf(sizeRatio);

        // Ajuster radius selon HP
        mRadius = 10.0f + (mStats.hp / 10.0f);
    }

    void OnCollide(Entity* other) override {
        // Juste pour log (degats appliques dans WorkerThread)
        // On ne fait rien ici (collision geree par thread)
    }

    float CalculateDamage(Entity* other) const {
        // Degats = base damage * 1.25 si plus gros
        float damageMultiplier = 1.0f;
        if (mStats.hp > other->mStats.hp) {
            damageMultiplier = 1.25f;
        }
        return mStats.damage * damageMultiplier;
    }

    // Rebond elastique sur autre boule
    void BounceOff(Ball* other) {
        // Direction de collision
        Vector2 collisionNormal = (other->mPos - mPos).Normalized();

        // Reflechir direction
        mDirection = mDirection.Reflect(collisionNormal);

        // Separer les boules (eviter overlap)
        float dist = (mPos - other->mPos).Length();
        float overlap = (mRadius + other->mRadius) - dist;
        if (overlap > 0) {
            mPos = mPos - collisionNormal * (overlap / 2.0f);
        }
    }
};
