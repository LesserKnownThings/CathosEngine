#pragma once

#include "Components/Transform.h"
#include "Math/Math.h"

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

// Helper functions used for transforms
namespace TransformSystem
{
constexpr fpm::fixed_16_16 TICKS_PER_SECOND(30.0f);
const fpm::fixed_16_16 SIM_DT = fpm::fixed_16_16(1.0f) / TICKS_PER_SECOND;

constexpr float NON_SIM_TICKS_PER_SECOND = 120.0f;
const float NON_SIM_DT = 1.0f / NON_SIM_TICKS_PER_SECOND;

inline void ExtractPosRot(const Mat4& mat, Float3& outPos, Quat& outRot)
{
    outPos = Float3(mat[3]);
    outRot = glm::quat_cast(mat);
}

inline void SetPosition(entt::registry& registry, entt::entity entity, const Float3& pos)
{
    auto& transform = registry.get<LocalTransform>(entity);
    transform.position = pos;
    transform.dirty = 1;
}

inline fpm::fixed_16_16 ToRad(fpm::fixed_16_16 degree)
{
    const fpm::fixed_16_16 scalar = fpm::fixed_16_16(180.0f);
    return degree * fpm::fixed_16_16::pi() / scalar;
}

inline Quat EulerToQuat(const Float3& eulersDegree)
{
    Float3 eulers = eulersDegree;

    // Convert to radians first
    eulers.x = ToRad(eulers.x);
    eulers.y = ToRad(eulers.y);
    eulers.z = ToRad(eulers.z);

    return Quat(eulers);
}

inline void SetRotation(entt::registry& registry, entt::entity entity, const Float3& eulers)
{
    auto& transform = registry.get<LocalTransform>(entity);

    transform.rotation = EulerToQuat(eulers);
    transform.dirty = 1;
}

inline void SetRotation(entt::registry& registry, entt::entity entity, const Quat& quat)
{
    auto& transform = registry.get<LocalTransform>(entity);
    transform.rotation = quat;
    transform.dirty = 1;
}

inline void Rotate(LocalTransform& transform, const Float3 axis, fpm::fixed_16_16 angle)
{
    auto axisNorm = Math::Normalize(axis);
    fpm::fixed_16_16 angleRad = ToRad(angle);

    auto sinA = fpm::sin(angleRad * fpm::fixed_16_16(0.5f));
    auto cosA = fpm::cos(angleRad * fpm::fixed_16_16(0.5f));

    Quat newRot = Quat(
        cosA,
        axisNorm.x * sinA,
        axisNorm.y * sinA,
        axisNorm.z * sinA);

    transform.rotation = Math::Normalize(newRot * transform.rotation);
    transform.dirty = 1;
}
} // namespace TransformSystem