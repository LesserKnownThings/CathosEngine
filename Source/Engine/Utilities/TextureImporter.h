#pragma once

#include <functional>
#include <string>

struct Texture2D;
struct TextureData;

class TextureImporter
{
  public:
    // Care this only supports RGBA, so if you try to import RGB it will give out an error
    static bool ImportTexture(const std::string& path, Texture2D* outTexture);
    static bool ReadPixels(const std::string& path, std::function<void(void*, const TextureData&)> func);
};