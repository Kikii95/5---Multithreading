#pragma once
#include <cmath>

// Vecteur 2D pour positions et directions
struct Vector2 {
    float x, y;

    Vector2() : x(0), y(0) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}

    // Operations
    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }

    Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    Vector2 operator/(float scalar) const {
        return Vector2(x / scalar, y / scalar);
    }

    // Magnitude (longueur du vecteur)
    float Length() const {
        return sqrtf(x * x + y * y);
    }

    // Normaliser (vecteur de longueur 1)
    Vector2 Normalized() const {
        float len = Length();
        if (len > 0.0001f) return Vector2(x / len, y / len);
        return Vector2(0, 0);
    }

    // Reflection (pour rebonds sur murs)
    Vector2 Reflect(const Vector2& normal) const {
        // Formule : R = V - 2 * (V . N) * N
        float dot = x * normal.x + y * normal.y;
        return Vector2(x - 2 * dot * normal.x, y - 2 * dot * normal.y);
    }
};
