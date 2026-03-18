#pragma once

#include "Resources/AssetPath.h"

struct TextureData;

class TextureImporter
{
  public:
    // Care this only supports RGBA, so if you try to import RGB it will give out an error
    static bool ImportTexture(const AssetPath& path, TextureData& outData);
};