#pragma once

#include "Windows/WindowBase.h"

#include <sstream>
#include <string>
#include <vector>

#define REGISTER_WINDOW(WindowType, Menu, IsVisible)                          \
    inline static struct Window_##WindowType                                  \
    {                                                                         \
        Window_##WindowType()                                                 \
        {                                                                     \
            WindowManager::Get().RegisterWindow<WindowType>(Menu, IsVisible); \
        }                                                                     \
    } instance_##WindowType;

struct EditorMenuNode
{
    std::string name;
    EditorMenuNode* child;
    EditorMenuNode* next;
    WindowBase* window;
};

class WindowManager
{
  public:
    ~WindowManager();
    static WindowManager& Get();

    const std::vector<WindowBase*>& GetWindows() const { return windows; }
    const std::vector<EditorMenuNode*>& GetMenuNodes() const { return menuNodes; }

    template <typename T>
    void RegisterWindow(const std::string& menu, bool isVisible);
    void DrawWindows();

  private:
    std::vector<WindowBase*> windows;
    std::vector<EditorMenuNode*> menuNodes;

    friend class EditorWorld;
};

inline EditorMenuNode* FindOrCreateNode(EditorMenuNode*& head, const std::string& name)
{
    if (!head)
    {
        head = new EditorMenuNode();
        head->name = name;
        return head;
    }

    EditorMenuNode* current = head;
    EditorMenuNode* prev = nullptr;

    while (current)
    {
        if (current->name == name)
            return current;

        prev = current;
        current = current->next;
    }

    prev->next = new EditorMenuNode();
    prev->next->name = name;

    return prev->next;
}

inline void AddMenuPath(EditorMenuNode*& root, const std::string& menu, WindowBase* window)
{
    std::stringstream ss(menu);

    std::string item;

    EditorMenuNode** currentList = &root;
    EditorMenuNode* currentNode = nullptr;

    while (std::getline(ss, item, '/'))
    {
        currentNode = FindOrCreateNode(*currentList, item);
        currentNode->window = window;
        currentList = &currentNode->child;
    }
}

template <typename T>
void WindowManager::RegisterWindow(const std::string& menu, bool isVisible)
{
    WindowBase* instance = new T();
    instance->Initialize(menu, isVisible);
    windows.push_back(instance);

    EditorMenuNode* root = nullptr;
    AddMenuPath(root, menu, instance);
    menuNodes.push_back(root);
}