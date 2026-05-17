#include "Font.h"
#include "Rendering/RenderingSystem.h"
#include "Resources/AssetPath.h"
#include "Resources/AssetTraits.h"
#include "Resources/Texture.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

Font::~Font()
{
    RenderingSystem::Get().DestroyTexture(atlasTexture);
}

Font::Font(const AssetPath& path)
{
    std::ifstream file(path.GetPath(), std::ios::binary);
    if (file.is_open())
    {
        uint32_t metadata;
        file.read(reinterpret_cast<char*>(&metadata), sizeof(uint32_t));

        if (metadata == AssetTraits<FontAssetMetadata>::id)
        {
            int32_t width = 0, height = 0;
            file.read(reinterpret_cast<char*>(&width), sizeof(int32_t));
            file.read(reinterpret_cast<char*>(&height), sizeof(int32_t));

            const int32_t pixelCount = width * height * 3;
            uint8_t* pixels = reinterpret_cast<uint8_t*>(malloc(pixelCount));
            file.read(reinterpret_cast<char*>(pixels), pixelCount);

            int32_t glyphsCount = 0;
            file.read(reinterpret_cast<char*>(&glyphsCount), sizeof(int32_t));

            if (glyphsCount > 0)
            {
                std::vector<GlyphData> glyphs(glyphsCount, GlyphData{});
                file.read(reinterpret_cast<char*>(&glyphs[0]), sizeof(GlyphData) * glyphsCount);

                for (const GlyphData& glyph : glyphs)
                {
                    mappedGlyphs.emplace(glyph.codePoint, glyph);
                }
            }

            int32_t kerningCount = 0;
            file.read(reinterpret_cast<char*>(&kerningCount), sizeof(int32_t));

            if (kerningCount > 0)
            {
                std::vector<KerningPair> kerningPairs(kerningCount, KerningPair{});
                file.read(reinterpret_cast<char*>(&kerningPairs[0]), sizeof(KerningPair) * kerningCount);

                for (const KerningPair& kerning : kerningPairs)
                {
                    mappedKerningPairs.emplace(std::pair{ kerning.left, kerning.right }, kerning.advanceAdjust);
                }
            }

            const int32_t pCount = width * height;
            uint8_t* rgbaPixels = reinterpret_cast<uint8_t*>(malloc(pCount * 4));
            for (int32_t i = 0; i < pCount; ++i)
            {
                rgbaPixels[i * 4 + 0] = pixels[i * 3 + 0];
                rgbaPixels[i * 4 + 1] = pixels[i * 3 + 1];
                rgbaPixels[i * 4 + 2] = pixels[i * 3 + 2];
                rgbaPixels[i * 4 + 3] = 255;
            }

            TextureData textureData{
                width,
                height,
                4,
                1,
                TextureFormat::RGBA_8_UNORM,
                TextureFilter::Linear,
            };
            RenderingSystem::Get().CreateTexture(textureData, rgbaPixels, atlasTexture, textureIndex);

            free(rgbaPixels);
            free(pixels);
        }

        file.close();
    }
}