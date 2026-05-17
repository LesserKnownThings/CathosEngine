#pragma once

#include "Rendering/VkData.h"
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <unordered_map>
#include <utility>

struct AssetPath;

struct GlyphData
{
    uint32_t codePoint;

    float advance;

    double planeLeft;
    double planeBottom;
    double planeRight;
    double planeTop;

    double uvLeft;
    double uvBottom;
    double uvRight;
    double uvTop;
};

struct KerningPair
{
    uint32_t left;
    uint32_t right;
    float advanceAdjust;
};

struct FontAssetMetadata
{
};

struct PairHash
{
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const
    {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

struct Font
{
    ~Font();
    Font(const AssetPath& path);

    AllocatedTexture atlasTexture;
    uint32_t textureIndex;
    std::unordered_map<uint32_t, GlyphData> mappedGlyphs;
    std::unordered_map<std::pair<uint32_t, uint32_t>, float, PairHash> mappedKerningPairs;
};