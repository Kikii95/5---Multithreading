#pragma once
#include "Entity.h"

// Types de bonus
enum class BonusType {
    HP_BOOST,       // +20 HP (taille)
    SPEED_BOOST,    // +20% speed temporaire
    SHIELD,         // Shield temporaire (immunité)
    DAMAGE_BOOST    // +50% damage temporaire
};

// Classe Bonus (entité ramassable statique)
class Bonus : public Entity {
public:
    BonusType mType;
    float mBoostValue;
    bool mIsPickedUp;

    // Constructeur
    Bonus(int id, Vector2 pos, BonusType type, float boostValue = 20.0f)
        : Entity(id, pos, 10.0f), // Radius fixe 10px
          mType(type), mBoostValue(boostValue), mIsPickedUp(false) {
        mStats.hp = 1.0f;       // Pas de HP (1 = symbole)
        mStats.speed = 0.0f;    // Statique
        mStats.damage = 0.0f;   // Pas de damage
        mDirection = Vector2(0, 0);
    }

    // Update : rien à faire (bonus statique)
    void Update(float deltaTime) override {
        // Bonus ne bouge pas
    }

    // OnCollide : rien (géré côté Player)
    void OnCollide(Entity* other) override {
        // Ne fait rien, le Player gère le pickup
    }

    // Marquer comme ramassé
    void PickUp() {
        mIsPickedUp = true;
        mIsDead = true;  // Supprimer du jeu
    }

    // Couleur selon type (pour rendering)
    int GetColorIndex() const {
        switch (mType) {
            case BonusType::HP_BOOST:     return 0; // Rouge
            case BonusType::SPEED_BOOST:  return 2; // Bleu
            case BonusType::SHIELD:       return 3; // Jaune
            case BonusType::DAMAGE_BOOST: return 4; // Violet
            default: return 0;
        }
    }
};
