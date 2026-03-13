#pragma once

#include "Math/Quaternions.h"
#include "Math/Vectors.h"
#include "fpm/fixed.hpp"
#include "fpm/math.hpp"

namespace Math
{
inline Float3 Normalize(const Float3& v)
{
    fpm::fixed_16_16 lengthSq = (v.x * v.x) + (v.y * v.y) + (v.z * v.z);

    if (lengthSq <= fpm::fixed_16_16(0.0f))
    {
        return Float3(fpm::fixed_16_16(0.0f));
    }

    fpm::fixed_16_16 invLength = fpm::fixed_16_16(1.0f) / fpm::sqrt(lengthSq);
    return Float3(v.x * invLength, v.y * invLength, v.z * invLength);
}

inline Quat Normalize(const Quat& q)
{
    fpm::fixed_16_16 lengthSq = (q.w * q.w) + (q.x * q.x) + (q.y * q.y) + (q.z * q.z);

    if (lengthSq <= fpm::fixed_16_16(0))
    {
        return Quat(
            fpm::fixed_16_16(1),
            fpm::fixed_16_16(0),
            fpm::fixed_16_16(0),
            fpm::fixed_16_16(0));
    }

    fpm::fixed_16_16 invLength = fpm::fixed_16_16(1) / fpm::sqrt(lengthSq);

    return Quat(
        q.w * invLength,
        q.x * invLength,
        q.y * invLength,
        q.z * invLength);
}
}; // namespace Math