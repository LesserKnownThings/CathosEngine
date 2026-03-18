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

bool TextureImporter::ImportTexture(const AssetPath& path, TextureData& outData)
{
    int32_t& width = outData.width;
    int32_t& height = outData.height;
    int32_t& channels = outData.channels;

    if (stbi_uc* pixels = stbi_load(path.GetPath().c_str(), &width, &height, &channels, STBI_rgb_alpha))
    {
        // TODO need to either manually calculate the mip levels or make sure this is correct
        outData.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        RenderingSystem::Get().CreateSRGBATexture(outData, pixels);
        return true;
    }

    LOG("TextureImporter", Error, std::format("Failed to import texture [{}]", path.GetPath()));
    return false;
}