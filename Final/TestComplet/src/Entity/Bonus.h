#pragma once
#include "Entity.h"
enum class BonusType {
    HP_BOOST,       
    SPEED_BOOST,    
    SHIELD,         
    DAMAGE_BOOST    
};
class Bonus : public Entity {
public:
    BonusType mType;
    float mBoostValue;
    bool mIsPickedUp;
    Bonus(int id, Vector2 pos, BonusType type, float boostValue = 20.0f)
        : Entity(id, pos, 10.0f), 
          mType(type), mBoostValue(boostValue), mIsPickedUp(false) {
        mStats.hp = 1.0f;       
        mStats.speed = 0.0f;    
        mStats.damage = 0.0f;   
        mDirection = Vector2(0, 0);
    }
    void Update(float deltaTime) override {
    }
    void OnCollide(Entity* other) override {
    }
    void PickUp() {
        mIsPickedUp = true;
        mIsDead = true;  
    }
    int GetColorIndex() const {
        switch (mType) {
            case BonusType::HP_BOOST:     return 0; 
            case BonusType::SPEED_BOOST:  return 2; 
            case BonusType::SHIELD:       return 3; 
            case BonusType::DAMAGE_BOOST: return 4; 
            default: return 0;
        }
    }
};
