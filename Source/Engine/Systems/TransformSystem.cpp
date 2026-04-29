#include "TransformSystem.h"
#include "Components/Transform.h"
#include "Math/TransformMath.hpp"
#include "Registry/CommandBuffer.h"
#include "Registry/Registry.h"
#include "Rendering/RenderingSystem.h"
#include "Systems/SystemRegistry.h"
#include "TaskScheduler.h"

REGISTER_SYSTEM(TransformSystem, SystemPhase::Simulation, DEPENDENCIES({}), DEPENDENCIES({}), 1);

TransformSystem::TransformSystem()
{
    SystemRegistry& registry = SystemRegistry::Get();
    registry.BindFunc<TransformSystem, &TransformSystem::Run>(this);
    registry.BindSyncFunc<TransformSystem, &TransformSystem::RunSync>(this);
}

void TransformSystem::Run(Registry* registry, CommandBuffer& cmd)
{
    TransformToRenderTransform(registry);
}

void TransformSystem::RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick)
{
    UpdateTransformHierarchy(registry);
}

void TransformSystem::UpdateTransformHierarchy(Registry* registry)
{
    entt::registry& reg = registry->Get();

    auto group = reg.group<LocalTransform, GlobalTransform, Hierarchy>();
    const entt::entity* storage = reg.storage<LocalTransform>().data();

    auto func = [&group, &storage](int32_t start, int32_t end)
    {
        for (int32_t i = start; i < end; ++i)
        {
            const auto entity = storage[i];

            LocalTransform& local = group.get<LocalTransform>(entity);

            if (local.dirty == 0)
            {
                return;
            }

            Hierarchy& h = group.get<Hierarchy>(entity);
            GlobalTransform& global = group.get<GlobalTransform>(entity);

            local.dirty = 0;

            if (h.parent == entt::null)
            {
                global.matrix = local.LocalMatrix();
            }
            else
            {
                GlobalTransform& parentGlobal = group.get<GlobalTransform>(h.parent);
                global.matrix = parentGlobal.matrix * local.LocalMatrix();
            }
        }
    };

    TaskScheduler::Get().ParallelForSync(group.size(), func);
}

void TransformSystem::TransformToRenderTransform(Registry* registry)
{
    entt::registry& reg = registry->Get();

    auto view = reg.view<RenderTransform, GlobalTransform>();
    auto& storage = reg.storage<RenderTransform>();

    const entt::entity* entities = storage.data();
    const int32_t size = storage.size();

    auto func = [&view, &entities](int32_t start, int32_t end)
    {
        for (int32_t i = start; i < end; ++i)
        {
            const GlobalTransform& global = view.get<GlobalTransform>(entities[i]);
            RenderTransform& render = view.get<RenderTransform>(entities[i]);

            render.prevPos = render.currentPos;
            render.prevRot = render.currentRot;
            render.prevScale = render.currentScale;

            TransformMath::ExtractPosScaleRot(global.matrix, render.currentPos, render.currentScale, render.currentRot);
        }
    };

    TaskScheduler::Get().ParallelForSync(size, func);
}