#pragma once

#include <entt/entt.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>

struct UIAnchor
{
    glm::vec2 min = glm::vec2(0.5f, 0.5f);
    glm::vec2 max = glm::vec2(0.5f, 0.5f);
};

struct UIPivot
{
    float x = 0.5f;
    float y = 0.5f;
};

// This is the transform that you modify at runtime to reposition the UI.
// The UI system then uses this, the UI anchor and UI pivot and calculates
// the UIRenderTransform
struct UITransform
{
    glm::vec2 position;
    glm::vec2 size;

    glm::vec2 scale = { 1.0f, 1.0f };

    int32_t localZOrder;
};

// The transform used by the renderer to draw, this is the actual position of the UI after
// the anchor and pivot math
struct UIRenderTransform
{
    glm::vec2 position;
    glm::vec2 size;

    bool Overlaps(const glm::vec2& other) const
    {
        const glm::vec2 end = position + size;
        return other.x >= position.x && other.x <= end.x && other.y >= position.y && other.y <= end.y;
    }
};

struct UIRenderOrder
{
    int32_t renderOrder;
};

struct ChildOf
{
    entt::entity entity = entt::null;
};

struct Children
{
    std::vector<entt::entity> children;
};