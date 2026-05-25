#pragma once

#include "Rendering/VkData.h"

struct AssetPath;

enum class TextureFormat
{
    RGBA_8_SRGB = 43,
    RGBA_8_UNORM = 37,
};

enum class TextureFilter
{
    Nearest = 0,
    Linear = 1,
};

struct TextureData
{
    int32_t width;
    int32_t height;
    int32_t channels;
    uint32_t mipLevels;

    TextureFormat format;
    TextureFilter filter;
};

struct Texture2D
{
    ~Texture2D();
    Texture2D(const AssetPath& path);
    Texture2D(uint8_t* pixels, int32_t width, int32_t height);

    TextureData data{};
    AllocatedTexture renderTexture;

    // Index in the descriptor buffer
    uint32_t textureIndex;
};