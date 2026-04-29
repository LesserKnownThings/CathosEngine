#include "World.h"

#include "Components/Collider.h"
#include "Components/Color.h"
#include "Components/Transform.h"
#include "Game/CommandProcessor.h"
#include "Game/Player.h"
#include "InputManager.h"
#include "Math/FixedMath.hpp"
#include "Netcode/NetworkManager.h"
#include "Registry/Registry.h"
#include "Rendering/Gizmos.h"
#include "Rendering/RenderingSystem.h"
#include "Resources/AssetPath.h"
#include "Systems/SystemRegistry.h"
#include "Utilities/SceneUtilities.h"
#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <glm/ext/vector_float2.hpp>

// inline void SpawnUnit(entt::registry& registry, )
// {
// }

bool World::Initialize(int argc, const char* argv[])
{
    // NetworkManager& netManager = NetworkManager::Get();
    // success &= netManager.Initialize(argc, argv);

    // Frontend::Get().Init(true);

    registry = new Registry();
    CreateWorld();

    SystemRegistry::Get().Init(registry, globalCmd);

    return true;
}

void World::Shutdown()
{
    delete player;
    delete registry;

    // NetworkManager::Get().Shutdown();
}

void World::Run()
{
    NetworkManager::Get().Run();
    InputManager::Get().PollInput();

    SystemRegistry::Get().Run(registry, globalCmd, SystemPhase::Simulation);

    // registry->Run();
    player->Run(simTick);
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
    SystemRegistry::Get().RunSync(registry, globalCmd, tick, SystemPhase::Simulation);
}

void World::Render(float alpha)
{
    RenderingSystem& rs = RenderingSystem::Get();

    for (int32_t i = 0; i < 100; ++i)
    {
        Gizmos::DrawCube(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f }, glm::vec3{ 0.0f } + (float)i * glm::vec3(2.0f, 0.0f, 0.0f) - glm::vec3(50.0f, 0.0f, 0.0f), glm::vec3(0.5f));
    }

    SystemRegistry::Get()
        .Run(registry, globalCmd, SystemPhase::Presentation);

    rs.BeginFrame();
    rs.Draw(registry, alpha);
    rs.EndFrame();
}

void World::CreateWorld()
{
    registry = new Registry();
    player = new Player(registry);

    // const fpm::fixed_16_16 row = fpm::fixed_16_16(i / 20);
    // const fpm::fixed_16_16 col = fpm::fixed_16_16(i % 20);

    // const Float3 rot = Float3(fpm::fixed_16_16(-90.0f), 0, 0);
    // const Float3 pos = Float3(row * fpm::fixed_16_16(1.1f), 0, col * fpm::fixed_16_16(1.1f));

    Transform tr1{
        Float3{},
        Float3{},
        Float3{ .5f }
    };

    SceneData data{
        AssetPath("Data/Meshes/cube.glb"),
        AssetPath{},
        Color::WHITE,
        false,
        Collider{
            1.0f,
            Float3{},
            Float3{},
            ColliderType::Sphere }
    };

    // SceneUtilities::CreateScene(registry, globalCmd, tr1, data);
}

void World::EndFrameCommandBuffer()
{
    globalCmd.Execute(registry);
}

void World::GCPass()
{
    registry->GCPass();
}
