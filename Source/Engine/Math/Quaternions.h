#pragma once

#include "fpm/fixed.hpp"
#include "fpm/math.hpp"

#include <glm/detail/qualifier.hpp>

namespace glm
{
inline fpm::fixed_16_16 inversesqrt(fpm::fixed_16_16 x)
{
    return fpm::fixed_16_16(1.0f) / fpm::sqrt(x);
}

inline fpm::fixed_16_16 sin(fpm::fixed_16_16 x)
{
    return fpm::sin(x);
}

inline fpm::fixed_16_16 cos(fpm::fixed_16_16 x)
{
    return fpm::cos(x);
}

inline fpm::fixed_16_16 sqrt(fpm::fixed_16_16 x)
{
    return fpm::sqrt(x);
}

inline fpm::fixed_16_16 acos(fpm::fixed_16_16 x)
{
    return fpm::acos(x);
}

inline fpm::fixed_16_16 atan2(fpm::fixed_16_16 y, fpm::fixed_16_16 x)
{
    return fpm::atan2(y, x);
}

inline fpm::fixed_16_16 abs(fpm::fixed_16_16 x)
{
    return fpm::abs(x);
}

inline fpm::fixed_16_16 max(fpm::fixed_16_16 a, fpm::fixed_16_16 b)
{
    return (a > b) ? a : b;
}

inline fpm::fixed_16_16 min(fpm::fixed_16_16 a, fpm::fixed_16_16 b)
{
    return (a < b) ? a : b;
}

template <qualifier Q>
GLM_FUNC_QUALIFIER qua<fpm::fixed_16_16, Q> quat_cast(mat<4, 4, fpm::fixed_16_16, Q> const& m)
{
    typedef fpm::fixed_16_16 T;

    T one(1);
    T zero(0);
    T quart(0.25f);

    T tr = one + m[0][0] + m[1][1] + m[2][2];

    if (tr > zero)
    {
        T s = sqrt(tr) * T(2);
        return qua<T, Q>::wxyz(
            quart * s,
            (m[1][2] - m[2][1]) / s,
            (m[2][0] - m[0][2]) / s,
            (m[0][1] - m[1][0]) / s);
    }
    else if ((m[0][0] > m[1][1]) && (m[0][0] > m[2][2]))
    {
        T s = sqrt(one + m[0][0] - m[1][1] - m[2][2]) * T(2);
        return qua<T, Q>::wxyz(
            (m[1][2] - m[2][1]) / s,
            quart * s,
            (m[0][1] + m[1][0]) / s,
            (m[0][2] + m[2][0]) / s);
    }
    else if (m[1][1] > m[2][2])
    {
        T s = sqrt(one + m[1][1] - m[0][0] - m[2][2]) * T(2);
        return qua<T, Q>::wxyz(
            (m[2][0] - m[0][2]) / s,
            (m[0][1] + m[1][0]) / s,
            quart * s,
            (m[1][2] + m[2][1]) / s);
    }
    else
    {
        T s = sqrt(one + m[2][2] - m[0][0] - m[1][1]) * T(2);
        return qua<T, Q>::wxyz(
            (m[0][1] - m[1][0]) / s,
            (m[0][2] + m[2][0]) / s,
            (m[1][2] + m[2][1]) / s,
            quart * s);
    }
}
} // namespace glm

#include <glm/gtc/quaternion.hpp>

typedef glm::qua<fpm::fixed_16_16> Quat;
