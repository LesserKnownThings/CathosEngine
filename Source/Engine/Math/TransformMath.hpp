#pragma once

#include "Components/Transform.h"
#include "Math/FixedMath.hpp"
#include "Math/Math.h"

namespace TransformMath
{
constexpr fpm::fixed_16_16 TICKS_PER_SECOND(30.0f);
const fpm::fixed_16_16 SIM_DT = fpm::fixed_16_16(1.0f) / TICKS_PER_SECOND;

constexpr float NON_SIM_TICKS_PER_SECOND = 120.0f;
const float NON_SIM_DT = 1.0f / NON_SIM_TICKS_PER_SECOND;

inline void ExtractPosScaleRot(const Mat4& mat, Float3& outPos, Float3& outScale, Quat& outRot)
{
    outPos = Float3(mat[3]);

    outScale.x = glm::length(Float3(mat[0]));
    outScale.y = glm::length(Float3(mat[1]));
    outScale.z = glm::length(Float3(mat[2]));

    glm::mat3 rotMat;
    const FixedT verySmallNumber(0.0001f);
    const FixedT one(1.0f);

    rotMat[0] = Float3(mat[0]) / (outScale.x > verySmallNumber ? outScale.x : one);
    rotMat[1] = Float3(mat[1]) / (outScale.y > verySmallNumber ? outScale.y : one);
    rotMat[2] = Float3(mat[2]) / (outScale.z > verySmallNumber ? outScale.z : one);

    outRot = glm::quat_cast(rotMat);
}

inline void SetPosition(entt::registry& registry, entt::entity entity, const Float3& pos)
{
    auto& transform = registry.get<LocalTransform>(entity);
    transform.position = pos;
    transform.dirty = 1;
}

inline FixedT ToRad(FixedT degree)
{
    const FixedT scalar(180.0f);
    return degree * FixedT::pi() / scalar;
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

inline void Rotate(LocalTransform& transform, const Float3 axis, FixedT angle)
{
    auto axisNorm = Math::Normalize(axis);
    FixedT angleRad = ToRad(angle);

    auto sinA = fpm::sin(angleRad * FixedT(0.5f));
    auto cosA = fpm::cos(angleRad * FixedT(0.5f));

    Quat newRot = Quat(
        cosA,
        axisNorm.x * sinA,
        axisNorm.y * sinA,
        axisNorm.z * sinA);

    transform.rotation = Math::Normalize(newRot * transform.rotation);
    transform.dirty = 1;
}
} // namespace TransformMath