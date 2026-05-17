#include "EditorOverlay.h"
#include "WindowManager.h"

#include <cmath>
#include <imgui.h>

inline void DrawMenuNode(const EditorMenuNode* node)
{
    while (node)
    {
        if (node->child)
        {
            if (ImGui::BeginMenu(node->name.c_str()))
            {
                DrawMenuNode(node->child);
                ImGui::EndMenu();
            }
        }
        else
        {
            if (ImGui::MenuItem(node->name.c_str()))
            {
                if (node->window)
                {
                    node->window->SetVisbility(true);
                }
            }
        }

        node = node->next;
    }
}

void EditorOverlay::Draw()
{
    ImGuiDockNodeFlags dockspace_flags =
        ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    windowFlags |= ImGuiWindowFlags_NoBackground;

    ImGui::Begin("FullscreenDockspaceOverlay", nullptr, windowFlags);

    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");

    ImGui::DockSpace(
        dockspace_id,
        ImVec2(0.0f, 0.0f),
        dockspace_flags);

    WindowManager& wm = WindowManager::Get();

    if (ImGui::BeginMainMenuBar())
    {
        for (const EditorMenuNode* menuNode : wm.GetMenuNodes())
        {
            DrawMenuNode(menuNode);
        }

        ImGui::EndMainMenuBar();
    }

    wm.DrawWindows();
    ImGui::End();
}
