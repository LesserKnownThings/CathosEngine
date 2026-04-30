#include "NavigationSystem.h"
#include "Map/MapFormat.h"
#include "Registry/Registry.h"
#include "Rendering/Gizmos.h"
#include "Resources/SpatialPartition/Sector.h"
#include "Systems/SystemRegistry.h"
#include "Utilities/MapUtils.h"
#include <cstdint>

REGISTER_SYSTEM(NavigationSystem, SystemPhase::Simulation, DEPENDENCIES({}), DEPENDENCIES({}), 0);

NavigationSystem::NavigationSystem()
{
    SystemRegistry& sreg = SystemRegistry::Get();
    sreg.BindInitFunc<NavigationSystem, &NavigationSystem::Init>(this);
    sreg.BindFunc<NavigationSystem, &NavigationSystem::Run>(this);
}

void NavigationSystem::Init(Registry* registry, CommandBuffer& cmd)
{
    MapFormat map{};
    if (MapUtils::ImportMap("Data/Maps/TestMap.cmf", map))
    {
        registry->AddResource(std::move(map));
    }
}

void NavigationSystem::Run(Registry* registry, CommandBuffer& cmd)
{
    if (registry->ContainsResource<MapFormat>())
    {
        const MapFormat map = registry->GetResource<MapFormat>();
        const int wSectors = map.width / SECTOR_DIM;
        const int hSectors = map.height / SECTOR_DIM;

        for (int32_t y = 0; y < hSectors; ++y)
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