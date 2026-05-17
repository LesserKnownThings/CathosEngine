#include "Texture.h"
#include "Rendering/RenderingSystem.h"
#include "Resources/AssetPath.h"
#include "Utilities/TextureImporter.h"

Texture2D::~Texture2D()
{
    RenderingSystem::Get().DestroyTexture(renderTexture);
}

Texture2D::Texture2D(const AssetPath& path)
{
    TextureImporter::ImportTexture(path.GetPath(), this);
}

Texture2D::Texture2D(uint8_t* pixels, int32_t width, int32_t height)
{
    data.width = width;
    data.height = height;
    data.channels = 4;
    data.format = TextureFormat::RGBA_8_SRGB,
    data.filter = TextureFilter::Nearest;

    RenderingSystem::Get().CreateTexture(data, pixels, renderTexture, textureIndex);
}