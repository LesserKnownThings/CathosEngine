#pragma once

#include <cstdint>
#include <type_traits>
#define GLM_FORCE_PURE
#define GLM_FORCE_INLINE

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

#include <glm/fwd.hpp>

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