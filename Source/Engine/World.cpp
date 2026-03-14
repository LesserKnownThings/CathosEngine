#include "World.h"

#include "Components/Transform.h"
#include "Game/Camera.h"
#include "Game/CommandProcessor.h"
#include "Game/Player.h"
#include "InputManager.h"
#include "Math/Vectors.h"
#include "Netcode/NetworkManager.h"
#include "Rendering/RenderingSystem.h"
#include "Resources/AssetPath.h"
#include "Resources/AssetServer.h"
#include "Systems/SceneSystem.h"
#include "Systems/TransformSystem.h"
#include "TaskScheduler.h"
#include "fpm/fixed.hpp"
#include <cstdint>
#include <entt/entity/fwd.hpp>

// inline void SpawnUnit(entt::registry& registry, )
// {
// }

inline void CleanupHierarchy(entt::registry& registry, entt::entity parent)
{
    const auto& hierarchy = registry.get<Hierarchy>(parent);

    entt::entity current = hierarchy.firstChild;
    while (current != entt::null)
    {
        entt::entity next = registry.get<Hierarchy>(current).nextSibling;
        registry.destroy(current);

        current = next;
    }
}

bool World::Initialize(int argc, const char* argv[])
{
    bool success = true;

    // NetworkManager& netManager = NetworkManager::Get();
    // success &= netManager.Initialize(argc, argv);

    success &= RenderingSystem::Get().Initialize();

    // Frontend::Get().Init(true);

    CreateWorld();

    return success;
}

void World::Shutdown()
{
    NetworkManager::Get().Shutdown();
}

void World::Run()
{
    NetworkManager::Get().Run();
    InputManager::Get().PollInput(currentTick);
}

void World::NetPulse()
{
    if (player == nullptr)
        return;

    for (const auto& cmd : player->pendingCommands)
    {
        uint32_t cmdTick = cmd.tick;

        if (cmdTick > currentTick)
        {
            // Record command for current use
            // CommandProcessor::Get().AddCommand(cmd);
        }
        else
        {
            // We got a past msg we need to rollback
        }
    }

    player->pendingCommands.clear();
}

void World::RunSim(uint32_t tick)
{
    currentTick = tick;

    auto group = registry.group<LocalTransform, GlobalTransform, Hierarchy>();
    auto container = registry.storage<LocalTransform>().data();

    UpdateTransformHierarchy(container, group);

    auto func = [&](int32_t start, int32_t end)
    {
        for (int32_t i = start; i < end; ++i)
        {
            LocalTransform& transform = group.get<LocalTransform>(container[i]);
            TransformSystem::Rotate(transform, Float3(fpm::fixed_16_16(0.0f), fpm::fixed_16_16(1.0f), fpm::fixed_16_16(0.0f)), fpm::fixed_16_16(50.0f) * TransformSystem::SIM_DT);
        }
    };

    TaskScheduler::Get().ParallelForSync(group.size(), func);

    // for (const auto& cmd : CommandProcessor::Get().GetCommandsForTick(tick))
    // {
    //     // Apply commands
    // }
}

void World::Render(float alpha)
{
    RenderingSystem& rs = RenderingSystem::Get();

    rs.BeginFrame();
    // TODO parallelize the sync
    SyncSimTransformToRenderTransform();
    rs.Draw(registry, alpha);
    rs.EndFrame();
}

void World::CreateWorld()
{
    player = new Player();

    registry.on_destroy<Hierarchy>().connect<&CleanupHierarchy>();

    AssetServer& as = registry.ctx().emplace<AssetServer>();

    auto camEntity = registry.create();
    registry.emplace<Camera>(camEntity);
    registry.emplace<CameraTransform>(camEntity, CameraTransform{ .position = glm::vec3(0.f, 0.f, -5.f) });

    for (int32_t i = 0; i < 1; ++i)
    {
        // const fpm::fixed_16_16 row = fpm::fixed_16_16(i / 20);
        // const fpm::fixed_16_16 col = fpm::fixed_16_16(i % 20);

        //  const Float3 rot = Float3(fpm::fixed_16_16(-90.0f), fpm::fixed_16_16(0.f), fpm::fixed_16_16(0.0f));
        const Float3 pos = Float3(fpm::fixed_16_16(5.0f), fpm::fixed_16_16(0.0f), fpm::fixed_16_16(5.0f));

        entt::entity parent = SceneSystem::CreateScene(registry, AssetPath("Data/Meshes/sphere.glb"), pos, Float3{}, true);
    }

    for (int32_t i = 0; i < 1; ++i)
    {
        // const fpm::fixed_16_16 row = fpm::fixed_16_16(i / 20);
        // const fpm::fixed_16_16 col = fpm::fixed_16_16(i % 20);

        // const Float3 rot = Float3(fpm::fixed_16_16(-90.0f), fpm::fixed_16_16(0.f), fpm::fixed_16_16(0.0f));
        // const Float3 pos = Float3(row * fpm::fixed_16_16(3.0f), fpm::fixed_16_16(0.0f), col * fpm::fixed_16_16(3.0f));

        entt::entity parent = SceneSystem::CreateScene(registry, AssetPath("Data/Meshes/cube.glb"));
    }
}

void World::UpdateTransformHierarchy(const entt::entity* storage, auto& group)
{
    auto func = [&group, &storage](int32_t start, int32_t end)
    {
        for (int32_t i = start; i < end; ++i)
        {
            const auto entity = storage[i];

            LocalTransform& local = group.template get<LocalTransform>(entity);
            Hierarchy& h = group.template get<Hierarchy>(entity);
            GlobalTransform& global = group.template get<GlobalTransform>(entity);

            if (local.dirty == 0)
            {
                return;
            }

            local.dirty = 0;

            if (h.parent == entt::null)
            {
                global.matrix = local.LocalMatrix();
            }
            else
            {
                GlobalTransform& parentGlobal = group.template get<GlobalTransform>(h.parent);
                global.matrix = parentGlobal.matrix * local.LocalMatrix();
            }
        }
    };

    TaskScheduler::Get().ParallelForSync(group.size(), func);
}

void World::SyncSimTransformToRenderTransform()
{
    auto view = registry.view<GlobalTransform, RenderTransform>();

    auto func = [&](GlobalTransform& global, RenderTransform& render)
    {
        render.prevPos = render.currentPos;
        render.prevRot = render.currentRot;

        TransformSystem::ExtractPosRot(global.matrix, render.currentPos, render.currentRot);
    };

    view.each(func);
}

void World::GCPass()
{
    AssetServer& as = registry.ctx().get<AssetServer>();
    as.GCPass();
}
