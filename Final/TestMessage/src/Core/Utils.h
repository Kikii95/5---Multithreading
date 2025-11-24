#pragma once
#include "Vector2.h"
#include <cstdlib>
#include <ctime>
namespace Config {
    const float MAP_WIDTH = 800.0f;
    const float MAP_HEIGHT = 600.0f;
    const float CELL_SIZE = 200.0f;  
    const int NB_THREADS = 4;        
    const int NB_BALLS = 12;         
}
namespace Utils {
    inline float Distance(const Vector2& a, const Vector2& b) {
        return (b - a).Length();
    }
    inline float RandomFloat(float min, float max) {
        float r = (float)rand() / (float)RAND_MAX;
        return min + r * (max - min);
    }
    inline Vector2 RandomDirection() {
        float angle = RandomFloat(0, 6.28318f);  
        return Vector2(cosf(angle), sinf(angle));
    }
    inline void InitRandom() {
        srand((unsigned int)time(NULL));
    }
}
