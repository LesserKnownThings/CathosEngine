#include "World.h"

#include "Components/Color.h"
#include "Components/Transform.h"
#include "Game/Camera.h"
#include "Game/CommandProcessor.h"
#include "Game/Player.h"
#include "InputManager.h"
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
#include <glm/ext/vector_float2.hpp>

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
    // NetworkManager::Get().Shutdown();
}

void World::Run()
{
    NetworkManager::Get().Run();
    InputManager::Get().PollInput();

    player->Run(registry, simTick);
}

void World::NetPulse()
{
    if (player == nullptr)
        return;

    for (const auto& cmd : player->pendingCommands)
    {
        uint32_t cmdTick = cmd.tick;

        if (cmdTick > simTick)
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
    simTick = tick;

    player->RunSim(registry, tick);
    UpdateTransformHierarchy();

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
    registry.emplace<CameraTransform>(camEntity, CameraTransform{ .position = glm::vec3(0.f, 30.f, -5.f) }.LookAt(glm::vec3(0.0f)));
    registry.emplace<CameraGlobalTransform>(camEntity);

    for (int32_t i = 0; i < 200; ++i)
    {
        const fpm::fixed_16_16 row = fpm::fixed_16_16(i / 20);
        const fpm::fixed_16_16 col = fpm::fixed_16_16(i % 20);

        const Float3 rot = Float3(fpm::fixed_16_16(-90.0f), 0, 0);
        const Float3 pos = Float3(row * fpm::fixed_16_16(3.0f), 0, col * fpm::fixed_16_16(3.0f));

        SceneSystem::SceneData data{
            AssetPath("Data/Meshes/sphere.glb"),
            AssetPath{},
            Color::YELLOW,
            pos,
            rot
        };

        entt::entity parent = SceneSystem::CreateScene(registry, data);
    }

    for (int32_t i = 0; i < 200; ++i)
    {
        const fpm::fixed_16_16 row = fpm::fixed_16_16(i / 20);
        const fpm::fixed_16_16 col = fpm::fixed_16_16(i % 20);

        const Float3 rot = Float3(fpm::fixed_16_16(-90.0f), 0, 0);
        const Float3 pos = Float3(row * fpm::fixed_16_16(3.0f), 0, col * fpm::fixed_16_16(3.0f)) + Float3(40.f, 0.f, 0.f);

        SceneSystem::SceneData data{
            AssetPath("Data/Meshes/cube.glb"),
            AssetPath{},
            Color::BLUE,
            pos,
            rot
        };

        entt::entity parent = SceneSystem::CreateScene(registry, data);
    }

    for (int32_t i = 0; i < 200; ++i)
    {
        const fpm::fixed_16_16 row = fpm::fixed_16_16(i / 20);
        const fpm::fixed_16_16 col = fpm::fixed_16_16(i % 20);

        const Float3 rot = Float3(fpm::fixed_16_16(-90.0f), 0, 0);
        const Float3 pos = Float3(row * fpm::fixed_16_16(3.0f), 0, col * fpm::fixed_16_16(3.0f)) + Float3(20.f, 0.f, 0.f);

        SceneSystem::SceneData data{
            AssetPath("Data/Meshes/cube.glb"),
            AssetPath{},
            Color::BLACK,
            pos,
            rot
        };

        entt::entity parent = SceneSystem::CreateScene(registry, data);
    }
}

void World::UpdateTransformHierarchy()
{
    auto group = registry.group<LocalTransform, GlobalTransform, Hierarchy>();
    const entt::entity* storage = registry.storage<LocalTransform>().data();

    auto func = [&group, &storage](int32_t start, int32_t end)
    {
        for (int32_t i = start; i < end; ++i)
        {
            const auto entity = storage[i];

            LocalTransform& local = group.get<LocalTransform>(entity);
            Hierarchy& h = group.get<Hierarchy>(entity);
            GlobalTransform& global = group.get<GlobalTransform>(entity);

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
                GlobalTransform& parentGlobal = group.get<GlobalTransform>(h.parent);
                global.matrix = parentGlobal.matrix * local.LocalMatrix();
            }
        }
    };

    TaskScheduler::Get().ParallelForSync(group.size(), func);

    // Updating the cameras here as well
    auto camGroup = registry.group<CameraTransform, CameraGlobalTransform>(entt::get<Camera>);
    const float aspectRatio = RenderingSystem::Get().GetAspectRatio();

    auto updateCamTransform = [&camGroup, &aspectRatio](CameraTransform& transform, CameraGlobalTransform& global, const Camera& cam)
    {
        if ((transform.flags & PROJECTION_CHANGED) != 0)
        {
            transform.flags &= ~PROJECTION_CHANGED;
            global.projection = Camera::CalculateProjection(cam, aspectRatio);
            global.projectionView = global.projection * global.view;
        }

        if ((transform.flags & VIEW_CHANGED) != 0)
        {
            transform.flags &= ~VIEW_CHANGED;

            transform.right = Camera::Right(transform);
            transform.forward = Camera::Forward(transform);
            transform.up = Camera::Up(transform);

            global.view = Camera::CalculateView(transform);
            global.projectionView = global.projection * global.view;
        }
    };

    camGroup.each(updateCamTransform);
}

void World::SyncSimTransformToRenderTransform()
{
    auto view = registry.view<RenderTransform, GlobalTransform>();
    auto& storage = registry.storage<RenderTransform>();

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

            TransformSystem::ExtractPosRot(global.matrix, render.currentPos, render.currentRot);
        }
    };

    TaskScheduler::Get().ParallelForSync(size, func);
}

void World::GCPass()
{
    AssetServer& as = registry.ctx().get<AssetServer>();
    as.GCPass();
}
