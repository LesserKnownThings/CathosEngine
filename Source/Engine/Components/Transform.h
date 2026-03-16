#pragma once

#include "Math/FixedMath.hpp"
#include <entt/entt.hpp>

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

// Used to interpolate the sim positons to render positions
struct RenderTransform
{
    Quat prevRot;
    Quat currentRot;

    Float3 prevPos;
    Float3 currentPos;
};

struct Hierarchy
{
    entt::entity parent = entt::null;
    entt::entity firstChild = entt::null;
    entt::entity nextSibling = entt::null;
    int32_t depth = 0;

    static void LinkEntities(entt::registry& registry, entt::entity child, entt::entity parent)
    {
        auto& childH = registry.get<Hierarchy>(child);
        auto& parentH = registry.get<Hierarchy>(parent);

        childH.parent = parent;

        if (parentH.firstChild != entt::null)
        {
            childH.nextSibling = parentH.firstChild;
        }

        parentH.firstChild = child;
    }
};
