#pragma once

#include <cstdint>

enum class ETextureFormat : uint8_t
{
    R8G8B8A8_SRGB = 43,
    R8_UINT = 13,
    R8_UNORM = 9,
    R32_UINT = 98,
};

enum class ETextureType : uint8_t
{
    TEXTURE_1D = 0,
    TEXTURE_2D = 1,
    TEXTURE_2D_ARRAY = 5,
};

struct TextureData
{
    int32_t width = 0;
    int32_t height = 0;
    int32_t channels = 0;
    int32_t layers = 1;
    ETextureFormat format = ETextureFormat::R8G8B8A8_SRGB;
    ETextureType viewType = ETextureType::TEXTURE_2D;
};

struct Texture;