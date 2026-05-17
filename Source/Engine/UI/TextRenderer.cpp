#include "TextRenderer.h"
#include "Resources/Font.h"

TextRenderer::TextRenderer(entt::resource<Font> inFont, const std::string& inText, float inFontSize)
    : font(inFont), text(inText), fontSize(inFontSize)
{
}