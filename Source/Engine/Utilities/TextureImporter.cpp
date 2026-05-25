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
    // TODO maybe flip the text texture instead? I'm flipping this because I made the text pipeline first and the msdf atlas generator
    // creates flipped y textures, so it's easier just to import the textures flipped
    stbi_set_flip_vertically_on_load(true);

    int32_t& width = outTexture->data.width;
    int32_t& height = outTexture->data.height;
    int32_t& channels = outTexture->data.channels;

    void* pixels = nullptr;

    const char* cPath = path.c_str();

    if (stbi_is_hdr(cPath))
    {
        pixels = stbi_loadf(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        // TODO set texture format
    }
    else if (stbi_is_16_bit(cPath))
    {
        pixels = stbi_load_16(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        // TODO set texture format
    }
    else
    {
        pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        outTexture->data.format = TextureFormat::RGBA_8_SRGB;
    }

    if (pixels != nullptr)
    {
        outTexture->data.filter = TextureFilter::Linear;
        // TODO need to either manually calculate the mip levels or make sure this is correct
        outTexture->data.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        RenderingSystem::Get().CreateTexture(outTexture->data, static_cast<uint8_t*>(pixels), outTexture->renderTexture, outTexture->textureIndex);
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