#pragma once
#include "../Core/Vector2.h"
#include "../Core/Utils.h"

// Stats de l'entite
struct Stats {
    float hp;
    float speed;
    float damage;

    Stats() : hp(100.0f), speed(80.0f), damage(15.0f) {}
    Stats(float _hp, float _speed, float _damage)
        : hp(_hp), speed(_speed), damage(_damage) {}
};

// Classe de base Entity (abstraite)
class Entity {
public:
    // Attributs
    int mID;                    // ID unique
    Vector2 mPos;               // Position centrale
    Vector2 mDirection;         // Direction mouvement (normalized)
    float mRadius;              // Rayon collision
    Stats mStats;               // Stats (HP, speed, damage)

    int mThreadIndex;           // Thread proprietaire
    bool mIsDead;               // Mort ?
    bool mIsTransitioning;      // En transition ? (immunite)

    // Constructeur
    Entity(int id, Vector2 pos, float radius)
        : mID(id), mPos(pos), mRadius(radius),
          mThreadIndex(-1), mIsDead(false), mIsTransitioning(false) {
        mDirection = Vector2(0, 0);
    }

    virtual ~Entity() {}

    // Methodes virtuelles
    virtual void Update(float deltaTime) = 0;
    virtual void OnCollide(Entity* other) = 0;

    // Detection collision (cercle vs cercle)
    bool Collide(Entity* other) const {
        if (!other || other == this) return false;
        if (mIsDead || other->mIsDead) return false;
        if (mIsTransitioning || other->mIsTransitioning) return false;

        float dist = (mPos - other->mPos).Length();
        return dist < (mRadius + other->mRadius);
    }

    // Serialisation (pour transfer entre threads)
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
