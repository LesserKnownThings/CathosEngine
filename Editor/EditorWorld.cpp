#include "EditorWorld.h"
#include "Callme/CallMe.h"
#include "Game/Player.h"
#include "InputManager.h"
#include "Map/MapFormat.h"
#include "Registry/Registry.h"
#include "Rendering/Gizmos.h"
#include "Rendering/RenderingSystem.h"
#include "Resources/Navigation/PathNetwork.h"
#include "Systems/SystemRegistry.h"
#include "WindowManager.h"
#include "Windows/EditorOverlay.h"
#include "Windows/HeightmapEditor.h"

#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <glm/ext/vector_float4.hpp>
#include <imgui.h>

bool EditorWorld::showMapPortals = false;
bool EditorWorld::showMapSectors = false;
bool EditorWorld::showPathNetwork = false;

void EditorWorld::Init()
{
    RenderingSystem& rs = RenderingSystem::Get();
    rs.InitImGui();

    static auto handle = HeightmapEditor::Get().onMapCreated.subscribe(CallMe::fromMethod<&EditorWorld::HandleMapCreated>(this));

    registry = new Registry();
    player = new Player(registry);

    SystemRegistry::Get().Init(registry, globalCmd);
}

void EditorWorld::HandleMapCreated(const MapFormat& map)
{
    if (!registry->ContainsResource<MapFormat>())
        registry->AddResource(std::move(map));
}

void EditorWorld::Run()
{
    InputManager::Get().PollInput();

    if (registry->ContainsResource<MapFormat>())
    {
        const MapFormat& map = registry->GetResource<MapFormat>();

        const int wSectors = map.width / SECTOR_DIM;
        const int hSectors = map.height / SECTOR_DIM;

        if (showMapSectors)
        {
            for (int32_t y = 0; y < hSectors; ++y)
            {
                for (int32_t x = 0; x < wSectors; ++x)
                {
                    const int32_t sectorIndex = y * wSectors + x;
                    const SectorData& sector = map.sectors[sectorIndex];
                    const glm::vec3 sectorPos = glm::vec3(x * SECTOR_DIM, 0.f, y * SECTOR_DIM);
                    if (!sector.hasCost)
                    {
                        const glm::vec3 half = glm::vec3(SECTOR_DIM / 2, 0.5f, SECTOR_DIM / 2);
                        Gizmos::DrawCube(glm::vec4(0.0, 1.0, 0.0, 1.0), sectorPos + half, half);
                    }
                    else
                    {
                        for (int32_t i = 0; i < SECTOR_DIM; ++i)
                        {
                            for (int32_t j = 0; j < SECTOR_DIM; ++j)
                            {
                                const int32_t cellIndex = i * SECTOR_DIM + j;
                                const uint8_t value = sector.costBuffer[cellIndex];

                                glm::vec4 color = glm::vec4(0.0, 1.0, 0.0, 1.0);

                                if (value == COST_WALL)
                                {
                                    color = glm::vec4(1.0, 0.0, 0.0, 1.0);
                                }
                                else if (value != COST_CONSTANT)
                                {
                                    color = glm::vec4(0.0, 0.0, 1.0, 1.0);
                                }

                                const glm::vec3 offset = sectorPos + glm::vec3(j, 0.0f, i) + glm::vec3(0.5f, 0.0f, 0.5f);
                                Gizmos::DrawCube(color, offset, glm::vec3(0.5f));
                            }
                        }
                    }
                }
            }
        }

        if (showMapPortals)
        {

            for (int32_t i = 0; i < map.portals.size(); ++i)
            {
                const Portal& portal = map.portals[i];
                const SectorData& a = map.sectors[portal.sector];

                const int32_t sectorsPerRow = map.width / SECTOR_DIM;

                const int32_t x = (portal.sector % sectorsPerRow) * SECTOR_DIM;
                const int32_t y = (portal.sector / sectorsPerRow) * SECTOR_DIM;

                const glm::vec3 sectorPos = glm::vec3(x, 0.0f, y);

                // Horizontal
                if (portal.direction == PortalDirection::Horizontal)
                {
                    for (int32_t i = portal.start; i <= portal.end; ++i)
                    {
                        const glm::vec3 pos = glm::vec3(i, 0.0f, SECTOR_DIM - 1.0f) + sectorPos + glm::vec3(0.5f, 0.0f, 0.5f);

                        Gizmos::DrawCube(glm::vec4(1.0, 1.0, 1.0, 1.0), pos, glm::vec3(0.5f));
                    }
                }
                else
                {
                    for (int32_t i = portal.start; i <= portal.end; ++i)
                    {
                        const glm::vec3 pos = glm::vec3(SECTOR_DIM - 1, 0.0f, i) + sectorPos + glm::vec3(0.5f, 0.0f, 0.5f);
                        Gizmos::DrawCube(glm::vec4(1.0, 1.0, 1.0, 1.0), pos, glm::vec3(0.5f));
                    }
                }
            }
        }

        if (showPathNetwork)
        {
            const PathNetworkMap& paths = registry->GetResource<PathNetworkMap>();
            for (auto& path : paths.networks)
            {
                if (path.second.path.size() < 1)
                    continue;

                for (int32_t i = 1; i < path.second.path.size() - 1; ++i)
                {
                    int32_t ax = path.second.path[i - 1] % map.HorizontalSectors();
                    int32_t az = path.second.path[i - 1] / map.HorizontalSectors();

                    int32_t bx = path.second.path[i] % map.HorizontalSectors();
                    int32_t bz = path.second.path[i] / map.HorizontalSectors();

                    Gizmos::DrawLine(glm::vec4(1.0f), glm::vec3(ax, 0.0f, az) * (float)SECTOR_DIM, glm::vec3(bx, 0.0f, bz) * (float)SECTOR_DIM);
                }
            }
        }
    }

    SystemRegistry& sysRegistry = SystemRegistry::Get();
    sysRegistry.Run(registry, globalCmd, SystemPhase::Simulation);
    sysRegistry.RunSync(registry, globalCmd, 0, SystemPhase::Simulation);
    player->Run(0);
}

EditorOverlay mainOverlay{};

void EditorWorld::Render()
{
    RenderingSystem& rs = RenderingSystem::Get();

    SystemRegistry::Get().Run(registry, globalCmd, SystemPhase::Presentation);

    rs.BeginFrame();

    // rs.Render3D(registry, 0.1f);
    rs.RenderUI(registry, [&]()
                { mainOverlay.Draw(); });

    rs.EndFrame();
}

void EditorWorld::EndFrameCommandBuffer()
{
    globalCmd.Execute(registry);
}
