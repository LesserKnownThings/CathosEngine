#pragma once

#include <cstdint>
#include <type_traits>
#define GLM_FORCE_PURE
#define GLM_FORCE_INLINE

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

#include <glm/fwd.hpp>

// namespace glm
// {
// // We use explicit overloads rather than 'using' to avoid
// // the "address of overloaded function" error in GLM functors.
// inline fixed abs(fixed x) { return fpm::abs(x); }
// inline fixed sqrt(fixed x) { return fpm::sqrt(x); }
// inline fixed sin(fixed x) { return fpm::sin(x); }
// inline fixed cos(fixed x) { return fpm::cos(x); }
// inline fixed acos(fixed x) { return fpm::acos(x); }
// inline fixed atan2(fixed y, fixed x) { return fpm::atan2(y, x); }
// inline fixed inversesqrt(fixed x) { return fixed(1) / fpm::sqrt(x); }

// // GLM's max/min usually conflict if not careful,
// // these concrete overloads for 'fixed' will take priority over templates.
// inline fixed max(fixed a, fixed b) { return (a > b) ? a : b; }
// inline fixed min(fixed a, fixed b) { return (a < b) ? a : b; }
// } // namespace glm

// #include <glm/detail/qualifier.hpp>

// namespace glm
// {
// template <glm::qualifier Q>
// GLM_FUNC_QUALIFIER glm::qua<fpm::fixed_16_16, Q> quat_cast(glm::mat<4, 4, fpm::fixed_16_16, Q> const& m)
// {
//     using T = fpm::fixed_16_16;

//     T one(1);
//     T zero(0);
//     T quart(0.25f);

//     T tr = one + m[0][0] + m[1][1] + m[2][2];

//     if (tr > zero)
//     {
//         T s = sqrt(tr) * T(2);
//         return glm::qua<T, Q>::wxyz(
//             quart * s,
//             (m[1][2] - m[2][1]) / s,
//             (m[2][0] - m[0][2]) / s,
//             (m[0][1] - m[1][0]) / s);
//     }
//     else if ((m[0][0] > m[1][1]) && (m[0][0] > m[2][2]))
//     {
//         T s = sqrt(one + m[0][0] - m[1][1] - m[2][2]) * T(2);
//         return glm::qua<T, Q>::wxyz(
//             (m[1][2] - m[2][1]) / s,
//             quart * s,
//             (m[0][1] + m[1][0]) / s,
//             (m[0][2] + m[2][0]) / s);
//     }
//     else if (m[1][1] > m[2][2])
//     {
//         T s = sqrt(one + m[1][1] - m[0][0] - m[2][2]) * T(2);
//         return glm::qua<T, Q>::wxyz(
//             (m[2][0] - m[0][2]) / s,
//             (m[0][1] + m[1][0]) / s,
//             quart * s,
//             (m[1][2] + m[2][1]) / s);
//     }
//     else
//     {
//         T s = sqrt(one + m[2][2] - m[0][0] - m[1][1]) * T(2);
//         return glm::qua<T, Q>::wxyz(
//             (m[0][1] - m[1][0]) / s,
//             (m[0][2] + m[2][0]) / s,
//             (m[1][2] + m[2][1]) / s,
//             quart * s);
//     }
// }
// } // namespace glm

template <typename BaseType, typename IntermediateType, uint8_t FractionBits>
struct GlmFixed : public fpm::fixed<BaseType, IntermediateType, FractionBits>
{
    using Base = fpm::fixed<BaseType, IntermediateType, FractionBits>;

    using Base::Base;

    GlmFixed() = default;
    GlmFixed(const Base& other) : Base(other) {}

    GlmFixed& operator=(const Base& other)
    {
        Base::operator=(other);
        return *this;
    }

    GlmFixed(int32_t val) : Base(val) {}
    GlmFixed(float val) : Base(val) {}
};

using FixedT = GlmFixed<int32_t, long, 16>;

namespace glm
{

template <typename T>
inline auto cos(T x) -> std::enable_if<std::is_floating_point_v<T>, T>
{
    return std::cos(x);
}

template <typename T>
inline auto sin(T x) -> std::enable_if<std::is_floating_point_v<T>, T>
{
    return std::sin(x);
}

inline FixedT cos(FixedT x) { return FixedT(fpm::cos(static_cast<fpm::fixed<int32_t, long, 16>>(x))); }
inline FixedT sin(FixedT x) { return FixedT(fpm::sin(static_cast<fpm::fixed<int32_t, long, 16>>(x))); }
inline FixedT tan(FixedT x) { return FixedT(fpm::tan(static_cast<fpm::fixed<int32_t, long, 16>>(x))); }

// Quaternions usually need acos and asin for slerp/angles
inline FixedT acos(FixedT x) { return FixedT(fpm::acos(static_cast<fpm::fixed<int32_t, long, 16>>(x))); }
inline FixedT asin(FixedT x) { return FixedT(fpm::asin(static_cast<fpm::fixed<int32_t, long, 16>>(x))); }

inline FixedT sqrt(FixedT x) { return FixedT(fpm::sqrt(static_cast<fpm::fixed<int32_t, long, 16>>(x))); }
inline FixedT atan2(FixedT y, FixedT x)
{
    return FixedT(fpm::atan2(static_cast<fpm::fixed<int32_t, long, 16>>(y), static_cast<fpm::fixed<int32_t, long, 16>>(x)));
}

inline FixedT abs(FixedT x) { return x < FixedT(0) ? -x : x; }
inline FixedT floor(FixedT x) { return FixedT(fpm::floor(static_cast<fpm::fixed<int32_t, long, 16>>(x))); }
inline FixedT ceil(FixedT x) { return FixedT(fpm::ceil(static_cast<fpm::fixed<int32_t, long, 16>>(x))); }
} // namespace glm

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

using Quat = glm::qua<FixedT>;

using Float2 = glm::vec<2, FixedT>;
using Float3 = glm::vec<3, FixedT>;
using Float4 = glm::vec<4, FixedT>;

using Mat3 = glm::mat<3, 3, FixedT>;
using Mat4 = glm::mat<4, 4, FixedT>;