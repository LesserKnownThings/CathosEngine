#pragma once

#include "Rendering/RenderingSystem.h"
#include "Rendering/VkData.h"
#include "Resources/AssetPath.h"
#include "Utilities/TextureImporter.h"

struct TextureData
{
    int32_t width;
    int32_t height;
    int32_t channels;
    uint32_t mipLevels;
};

struct Texture2D
{
    ~Texture2D()
    {
        RenderingSystem::Get().DestroyTexture(renderTexture);
    }

    Texture2D(const AssetPath& path)
    {
        TextureImporter::ImportTexture(path.GetPath(), this);
    }

    TextureData data;
    AllocatedTexture renderTexture;

    // Index in the descriptor buffer
    int32_t textureIndex;
};