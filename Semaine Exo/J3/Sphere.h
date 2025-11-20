#ifndef SPHERE_H
#define SPHERE_H

#include <math.h>

struct Point3D {
    float x;
    float y;
    float z;

    Point3D() : x(0.0f), y(0.0f), z(0.0f) {}
    Point3D(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

struct Sphere {
    Point3D center;
    float radius;
    int id;

    Sphere() : center(), radius(1.0f), id(0) {}
    Sphere(Point3D c, float r, int i) : center(c), radius(r), id(i) {}
};

// Calcule la distance entre deux points
inline float Distance3D(const Point3D& p1, const Point3D& p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float dz = p2.z - p1.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

// Verifie si deux spheres sont en collision
inline bool CheckCollision(const Sphere& s1, const Sphere& s2) {
    float distance = Distance3D(s1.center, s2.center);
    float sumRadius = s1.radius + s2.radius;
    return distance < sumRadius;
}

#endif // SPHERE_H
