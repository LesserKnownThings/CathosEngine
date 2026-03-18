#pragma once

#include "Components/Color.h"
#include "Resources/Texture.h"
#include <entt/resource/resource.hpp>
#include <glm/ext/vector_int4.hpp>

struct MaterialData
{
    glm::vec4 color;
    // Only x is valid here
    glm::ivec4 index;
};

// TDO support multiple shaders for materials, for now this supports only PBR
struct MaterialMesh3DHandle
{
    ~MaterialMesh3DHandle()
    {
    }

    MaterialMesh3DHandle(entt::resource<Texture2D> texture, const glm::vec4& inColor = Color::WHITE)
        : textureHandle(texture)
    {
    }

    entt::resource<Texture2D> textureHandle;
};

struct MaterialMesh3D
{
    entt::resource<MaterialMesh3DHandle> handle;
    glm::vec4 color;
};