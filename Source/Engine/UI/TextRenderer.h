#pragma once

#include <entt/resource/resource.hpp>
#include <glm/ext/vector_float4.hpp>
#include <string>

struct Font;

constexpr int32_t MAX_RENDER_GLYPSH = 10000;

enum class TextHAlign : uint8_t
{
    Left = 0,
    Center = 1,
    Right = 2,
    Justified = 3
};

enum class TextVAlign : uint8_t
{
    Top = 0,
    Middle = 1,
    Bottom = 2
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