#pragma once
#include <cmath>
struct Vector2 {
    float x, y;
    Vector2() : x(0), y(0) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}
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
    float Length() const {
        return sqrtf(x * x + y * y);
    }
    Vector2 Normalized() const {
        float len = Length();
        if (len > 0.0001f) return Vector2(x / len, y / len);
        return Vector2(0, 0);
    }
    Vector2 Reflect(const Vector2& normal) const {
        float dot = x * normal.x + y * normal.y;
        return Vector2(x - 2 * dot * normal.x, y - 2 * dot * normal.y);
    }
};
