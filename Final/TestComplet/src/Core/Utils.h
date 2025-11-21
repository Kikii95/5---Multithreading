#pragma once
#include "Vector2.h"
#include <cstdlib>
#include <ctime>

// Config globale
namespace Config {
    const float MAP_WIDTH = 800.0f;
    const float MAP_HEIGHT = 600.0f;
    const float CELL_SIZE = 200.0f;  // Taille cellule grille
    const int NB_THREADS = 4;        // Nombre de threads workers
    const int NB_PLAYERS = 12;       // Nombre de players initiaux
    const int NB_BONUS = 8;          // Nombre de bonus sur la map
}

// Utilitaires
namespace Utils {
    // Distance entre 2 points
    inline float Distance(const Vector2& a, const Vector2& b) {
        return (b - a).Length();
    }

    // Random float entre min et max
    inline float RandomFloat(float min, float max) {
        float r = (float)rand() / (float)RAND_MAX;
        return min + r * (max - min);
    }

    // Random direction normalisee
    inline Vector2 RandomDirection() {
        float angle = RandomFloat(0, 6.28318f);  // 0 à 2*PI
        return Vector2(cosf(angle), sinf(angle));
    }

    // Initialiser random seed
    inline void InitRandom() {
        srand((unsigned int)time(NULL));
    }
}
