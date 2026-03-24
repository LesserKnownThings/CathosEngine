#include "Menus.h"

#include "Utilities/portable-file-dialogs.h"
#include "Windows/HeightmapEditor.h"
#include <cmath>
#include <cstdint>
#include <imgui.h>
#include <vector>

constexpr const char* MAP_EDITOR_LOG = "MapEditor";
constexpr int32_t HEIGHT_SCALE = 100;
constexpr float MAX_SLOPE = 30.0f;

void Menus::Draw()
{
    static std::string heightmapPath;
    static std::string normalPath;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
    ImGui::Begin("Menus", nullptr, windowFlags);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Commands"))
        {
            if (ImGui::MenuItem("Export"))
            {
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::LabelText("Heightmap", "%s", heightmapPath.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Browse##1"))
    {
        auto result = pfd::open_file("Load heightmap texture.", "", { "Heightmap", "*.png" }).result();

        if (!result.empty())
        {
            heightmapPath = result[0];
        }
    }

    ImGui::LabelText("Normal", "%s", normalPath.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Browse##2"))
    {
        auto result = pfd::open_file("Load normal texture.", "", { "Normal", "*.png" }).result();

        if (!result.empty())
        {
            normalPath = result[0];
        }
    }

    if (!heightmapPath.empty() && !normalPath.empty())
    {
        if (ImGui::Button("Bake Map"))
        {
            HeightmapEditor::Get().Generate(heightmapPath, normalPath);
        }
    }

    ImGui::End();
}
