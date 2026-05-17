#pragma once

#include <entt/resource/resource.hpp>
#include <string>

struct Font;

constexpr int32_t MAX_RENDER_GLYPSH = 10000;

struct TextRenderer
{
    TextRenderer(entt::resource<Font> inFont, const std::string& inText, float inFontSize = 15.0f);

    std::string text;
    entt::resource<Font> font;

    float fontSize;
};