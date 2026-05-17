#include "WindowBase.h"

void WindowBase::Initialize(const std::string& inMenu, bool inVisibility)
{
    isShowing = inVisibility;
    if (!inMenu.empty())
    {
        menu = inMenu;
    }
}