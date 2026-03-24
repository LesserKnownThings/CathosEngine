#include "SpatialPartitionSystem.h"
#include "Components/SpatialData.h"
#include "Registry/Registry.h"
#include "Resources/AssetServer.h"
#include "Resources/SpatialPartition/Grid.h"
#include "Systems/SystemRegistry.h"
#include <atomic>
#include <entt/entt.hpp>
#include <vector>

REGISTER_SYSTEM(SpatialPartitionSystem, SystemPhase::Physics, DEPENDENCIES({}), DEPENDENCIES({}));

SpatialPartitionSystem::SpatialPartitionSystem()
{
    SystemRegistry& registry = SystemRegistry::Get();
    registry.BindSyncFunc<SpatialPartitionSystem, &SpatialPartitionSystem::RunSync>(this);
    registry.BindInitFunc<SpatialPartitionSystem, &SpatialPartitionSystem::Init>(this);
}

void SpatialPartitionSystem::Init(Registry* registry, CommandBuffer& cmd)
{
    entt::registry& reg = registry->Get();
    AssetServer& as = registry->GetAssetServer();

    // TODO load map format

    AtomicGrid& grid = registry->AddResource<AtomicGrid>(100, 100, 10000);

    for (int32_t y = 0; y < 100; ++y)
    {
        for (int32_t x = 0; x < 100; ++x)
        {
            const int32_t index = y * 100 + x;

            entt::entity cell = reg.create();
            reg.emplace<SpatialData>(cell, SpatialData{ index });
            grid.heads[index].store(cell, std::memory_order_relaxed);
        }
    }
}

void SpatialPartitionSystem::RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick)
{
}