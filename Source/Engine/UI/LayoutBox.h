#pragma once

#include <cstdint>
#include <glm/glm.hpp>

enum class ChildStart : uint8_t
{
    Start = 0,
    Middle = 1,
    End = 2,
};

struct VBox
{
    // LEFT RIGHT TOP BOTTOM
    glm::vec4 offset;

    float spacing;
    ChildStart childStart;

    bool controlHSize;
    bool controlVSize;
};

struct HBox
{
    // LEFT RIGHT TOP BOTTOM
    glm::vec4 offset;

    float spacing;
    ChildStart childStart;

    bool controlHSize;
    bool controlVSize;
};

struct GridBox
{
    // LEFT RIGHT TOP BOTTOM
    glm::vec4 offset;

    glm::vec2 size;
    glm::vec2 spacing;
    ChildStart childstart;
};
