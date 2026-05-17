#pragma once

#include <glm/glm.hpp>

struct UIInstanceData
{
    glm::vec2 position;
    glm::vec2 size;

    glm::vec4 uvRect;

    glm::vec4 color;

    uint32_t textureIndex;
};
