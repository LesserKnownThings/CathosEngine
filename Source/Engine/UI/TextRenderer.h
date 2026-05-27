#pragma once

#include <entt/resource/resource.hpp>
#include <glm/ext/vector_float4.hpp>
#include <string>

struct Font;

constexpr int32_t MAX_RENDER_GLYPSH = 10000;

enum class TextHAlign : uint8_t
{
    Left,
    Center,
    Right,
    Justified
};

enum class TextVAlign : uint8_t
{
    Top,
    Middle,
    Bottom
};

struct TextRenderer
{
    std::string text;
    glm::vec4 color;

    float fontSize;

    entt::resource<Font> font;
};

struct TextRendererDetails
{
    std::vector<std::string> words;
    std::vector<float> widths;
};

struct TextStyle
{
    float characterSpacing;
    float lineSpacing;

    TextHAlign horizontal;
    TextVAlign vertical;
    bool wrapText;
};