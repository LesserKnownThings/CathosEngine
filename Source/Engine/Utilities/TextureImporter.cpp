#include "TextureImporter.h"

#include "Debug/DebugSystem.h"
#include "Rendering/RenderingSystem.h"
#include "Resources/Texture.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool TextureImporter::ImportTexture(const std::string& path, Texture2D* outTexture)
{
    int32_t& width = outTexture->data.width;
    int32_t& height = outTexture->data.height;
    int32_t& channels = outTexture->data.channels;

    if (stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha))
    {
        // TODO need to either manually calculate the mip levels or make sure this is correct
        outTexture->data.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        RenderingSystem::Get().CreateSRGBATexture(outTexture, pixels);
        stbi_image_free(pixels);
        return true;
    }

    LOG("TextureImporter", Error, std::format("Failed to import texture [{}]", path));
    return false;
}

bool TextureImporter::ReadPixels(const std::string& path, std::function<void(void*, const TextureData&)> func)
{
    TextureData data{};
    int32_t& width = data.width;
    int32_t& height = data.height;
    int32_t& channels = data.channels;

    stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha))
    {
        func(pixels, data);
        stbi_image_free(pixels);
        return true;
    }

    LOG("TextureImporter", Error, std::format("Failed to import texture [{}]", path));
    return false;
}