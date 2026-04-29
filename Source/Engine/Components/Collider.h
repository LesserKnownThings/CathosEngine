#pragma once

#include "Math/FixedMath.hpp"
#include <cstdint>

enum class ColliderType : uint8_t
{
    // For static objects
    AABB,
    // For dynamic objects
    Sphere
};

struct Collider
{
    // Only valid if the collider type is Sphere
    FixedT radius;

    // Only valid if the collider type is AABB
    Float3 min;
    Float3 max;

    ColliderType type;
};
