#include "MapWorld.h"
#include "Callme/CallMe.h"
#include "Components/Color.h"
#include "Components/Transform.h"
#include "Game/Player.h"
#include "InputManager.h"
#include "Map/MapFormat.h"
#include "Registry/Registry.h"
#include "Rendering/RenderingSystem.h"
#include "Resources/SpatialPartition/Sector.h"
#include "Systems/SystemRegistry.h"
#include "Utilities/SceneUtilities.h"
#include "Windows/HeightmapEditor.h"
#include "Windows/Menus.h"

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <glm/ext/vector_float4.hpp>
#include <imgui.h>

void MapWorld::Init()
{
    windows.push_back(new Menus());

    RenderingSystem& rs = RenderingSystem::Get();
    rs.InitImGui();

    static auto handle = HeightmapEditor::Get().onMapCreated.subscribe(CallMe::fromMethod<&MapWorld::HandleMapCreated>(this));

    registry = new Registry();
    player = new Player(registry);

    SystemRegistry::Get().Init(registry, globalCmd);
}

void MapWorld::HandleMapCreated(const MapFormat& map)
{
    int32_t hSectors = map.height / SECTOR_DIM;
    int32_t wSectors = map.width / SECTOR_DIM;

    for (int32_t y = 0; y < hSectors; ++y)
    {
        for (int32_t x = 0; x < wSectors; ++x)
        {
            const int32_t index = y * wSectors + x;

            const SectorData& sector = map.sectors[index];

            for (int32_t cellY = 0; cellY < SECTOR_DIM; ++cellY)
            {
                for (int32_t cellX = 0; cellX < SECTOR_DIM; ++cellX)
                {
                    const Float3 pos{ x + cellX, 0.0f, y + cellY };
                    glm::vec4 color = Color::WHITE;

                    if (sector.costType != 0)
                    {
                        const int32_t cellIndex = cellY * SECTOR_DIM + cellX;
                        if (sector.costBuffer[cellIndex] == 255)
                        {
                            color = Color::RED;
                        }
                    }

                    Transform spatial{
                        pos,
                        Float3{},
                        Float3{ .5f }
                    };

                    SceneData data{
                        AssetPath("Data/Meshes/cube.glb"),
                        AssetPath{},
                        color,
                    };

                    SceneUtilities::CreateScene(registry, globalCmd, spatial, data);
                }
            }
        }
    }
}

void MapWorld::Run()
{
    InputManager::Get().PollInput();

    SystemRegistry& sysRegistry = SystemRegistry::Get();
    sysRegistry.Run(registry, globalCmd, SystemPhase::Logic);

    player->Run(0);
}

void MapWorld::Render()
{
    RenderingSystem& rs = RenderingSystem::Get();

    SystemRegistry::Get().Run(registry, globalCmd, SystemPhase::Presentation);

    rs.BeginFrame();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    for (auto window : windows)
    {
        window->Draw();
    }

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, rs.GetCurrentCommandBuffer());

    rs.Draw(registry->Get(), 0.01f);

    rs.EndFrame();
}

void MapWorld::EndFrameCommandBuffer()
{
    globalCmd.Execute(registry);
}
