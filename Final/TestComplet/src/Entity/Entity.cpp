#include "Entity.h"
#include "Ball.h"

Entity::SerializedData Entity::Serialize() const {
    SerializedData data;
    data.id = mID;
    data.pos = mPos;
    data.direction = mDirection;
    data.radius = mRadius;
    data.stats = mStats;
    data.isDead = mIsDead;

    // Si c'est une Ball, serialiser killCount
    const Ball* ball = dynamic_cast<const Ball*>(this);
    if (ball) {
        data.killCount = ball->mKillCount;
    } else {
        data.killCount = 0;
    }

    return data;
}

void Entity::Deserialize(const SerializedData& data) {
    mID = data.id;
    mPos = data.pos;
    mDirection = data.direction;
    mRadius = data.radius;
    mStats = data.stats;
    mIsDead = data.isDead;

    // Si c'est une Ball, deserialiser killCount
    Ball* ball = dynamic_cast<Ball*>(this);
    if (ball) {
        ball->mKillCount = data.killCount;
    }
}
