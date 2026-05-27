#pragma once

#include "Resources/Texture.h"
#include <entt/entt.hpp>
#include <glm/glm.hpp>

struct UIMaterial
{
    entt::resource<Texture2D> textureHandle;

    glm::vec4 color = glm::vec4(1.0f);
    glm::vec4 uv = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
};