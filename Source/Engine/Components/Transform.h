#pragma once

#include "Math/FixedMath.hpp"
#include <entt/entt.hpp>

struct Transform
{
    Float3 position;
    Float3 eulers;
    Float3 scale{ 1.0f };
};

struct LocalTransform
{
    Quat rotation;
    Float3 position;
    Float3 scale{ 1.0f };

    // Starts with true so we can compute the global matrix
    uint8_t dirty = 1;

    Mat4 LocalMatrix() const
    {
        return glm::translate(Mat4(1.0f), position) * glm::mat4_cast(rotation) * glm::scale(Mat4(1.0f), scale);
    }
};

struct GlobalTransform
{
    Mat4 matrix{ 1.0f };
};

// Used to interpolate the sim transform to render transform
struct RenderTransform
{
    Quat prevRot;
    Quat currentRot;

    Float3 prevPos;
    Float3 currentPos;
    Float3 prevScale;
    Float3 currentScale;
};

struct Hierarchy
{
    entt::entity parent = entt::null;
    entt::entity firstChild = entt::null;
    entt::entity nextSibling = entt::null;
    int32_t depth = 0;
};
