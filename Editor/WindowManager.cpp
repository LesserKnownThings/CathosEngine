#include "WindowManager.h"
#include "Windows/WindowBase.h"

WindowManager& WindowManager::Get()
{
    static WindowManager instance;
    return instance;
}

WindowManager::~WindowManager()
{
    for (EditorMenuNode* node : menuNodes)
    {
        delete node;
    }
    menuNodes.clear();
}

void WindowManager::DrawWindows()
{
    for (WindowBase* window : windows)
    {
        if (window->IsShowing())
        {
            window->Draw();
        }
    }
}
