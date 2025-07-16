#pragma once
#include <cmath>

class Vector3 {
public:
    float x, y, z;

    Vector3 operator-(const Vector3& other) const {
        return Vector3{ x - other.x, y - other.y, z - other.z };
    }

    float Length() const {
        return sqrt(x * x + y * y + z * z);
    }
};
