#include "SpatialPartitionSystem.h"
#include "Components/Collider.h"
#include "Components/SpatialData.h"
#include "Components/Transform.h"
#include "Map/MapFormat.h"
#include "Registry/Registry.h"
#include "Resources/AssetServer.h"
#include "Resources/SpatialPartition/Grid.h"
#include "Systems/SystemRegistry.h"
#include "Systems/TransformSystem.h"
#include "TaskScheduler.h"
#include "Utilities/MapUtils.h"
#include <atomic>
#include <entt/entity/entity.hpp>
#include <entt/entt.hpp>

REGISTER_SYSTEM(SpatialPartitionSystem, SystemPhase::Simulation, DEPENDENCIES({ typeid(TransformSystem) }), DEPENDENCIES({}), 0);

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

    MapFormat map{};
    if (MapUtils::ImportMap("Data/Maps/TestMap.cmf", map))
    {
        AtomicGrid& grid = registry->AddResource<AtomicGrid>(std::move(map));
    }
}

void SpatialPartitionSystem::RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick)
{
    AtomicGrid& grid = registry->GetResource<AtomicGrid>();

    entt::registry& reg = registry->Get();

    auto view = reg.view<LocalTransform, Collider, SpatialData>();

    const auto& storage = reg.storage<Collider>();
    const int32_t storageSize = storage.size();
    const entt::entity* entities = storage.data();

    for (size_t i = 0; i < grid.heads.size(); ++i)
    {
        grid.heads[i].store(entt::null, std::memory_order_relaxed);
    }

    TaskScheduler& ts = TaskScheduler::Get();

    auto processView = [&view, &grid, &entities](int32_t start, int32_t end)
    {
        for (int32_t i = start; i < end; ++i)
        {
            const LocalTransform& transform = view.get<LocalTransform>(entities[i]);
            SpatialData& spatial = view.get<SpatialData>(entities[i]);

            const Float3& pos = transform.position;
            const int32_t cellIndex = static_cast<int32_t>(pos.z) * grid.width + static_cast<int32_t>(pos.x);
            spatial.cell = cellIndex;

            auto& head = grid.heads[cellIndex];
            entt::entity oldHead = head.load(std::memory_order_relaxed);

            do
            {
                spatial.next = oldHead;
            } while (!head.compare_exchange_weak(
                oldHead,
                entities[i],
                std::memory_order_release,
                std::memory_order_relaxed));
        }
    };
    ts.ParallelForSync(storageSize, processView);
}