#pragma once

#include "Rendering/Pipelines/RenderPipeline.h"
#include "Resources/Texture.h"
#include <entt/entt.hpp>

struct UIMaterial
{
    entt::resource<Texture2D> textureHandle;

    glm::vec4 uv = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    glm::vec4 color = glm::vec4(1.0f);

    PipelineType pipeline;
};