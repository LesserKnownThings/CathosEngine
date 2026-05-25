#include "TransformSystem.h"
#include "Components/Transform.h"
#include "Math/TransformMath.hpp"
#include "Registry/CommandBuffer.h"
#include "Registry/Registry.h"
#include "Rendering/RenderingSystem.h"
#include "Systems/SystemRegistry.h"
#include "TaskScheduler.h"

REGISTER_SYSTEM(TransformSystem, SystemPhase::Simulation, DEPENDENCIES({}), DEPENDENCIES({}), 1);

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

    auto view = reg.view<LocalTransform, Hierarchy, GlobalTransform>();
    auto entities = std::vector<entt::entity>(view.begin(), view.end());

    auto func = [&view, &entities](int32_t start, int32_t end)
    {
        for (int32_t i = start; i < end; ++i)
        {
            auto [local, hierarchy, global] = view.get<LocalTransform, Hierarchy, const GlobalTransform>(entities[i]);

            if (local.dirty == 0)
            {
                return;
            }

            local.dirty = 0;

            if (hierarchy.parent == entt::null)
            {
                global.matrix = local.LocalMatrix();
            }
            else
            {
                GlobalTransform& parentGlobal = view.get<GlobalTransform>(hierarchy.parent);
                global.matrix = parentGlobal.matrix * local.LocalMatrix();
            }
        }
    };

    TaskScheduler::Get().ParallelForSync(entities.size(), func);
}

void TransformSystem::TransformToRenderTransform(Registry* registry)
{
    entt::registry& reg = registry->Get();

    auto view = reg.view<RenderTransform, GlobalTransform>();
    const auto entities = std::vector<entt::entity>(view.begin(), view.end());

    auto func = [&view, &entities](int32_t start, int32_t end)
    {
        for (int32_t i = start; i < end; ++i)
        {
            auto [render, global] = view.get<RenderTransform, const GlobalTransform>(entities[i]);

            render.prevPos = render.currentPos;
            render.prevRot = render.currentRot;
            render.prevScale = render.currentScale;

            TransformMath::ExtractPosScaleRot(global.matrix, render.currentPos, render.currentScale, render.currentRot);
        }
    };

    TaskScheduler::Get().ParallelForSync(entities.size(), func);
}