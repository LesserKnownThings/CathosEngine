#pragma once

#include <cstdint>

enum class VisibilityMode : uint8_t
{
    Visibile,
    Hidden,
    Collapsed,
};

struct UIVisiblity
{
    VisibilityMode mode = VisibilityMode::Visibile;
    bool isVisible = true;
};