#include "World.h"

#include "Components/Collider.h"
#include "Components/Color.h"
#include "Components/Transform.h"
#include "Game/CommandProcessor.h"
#include "Game/Player.h"
#include "InputManager.h"
#include "Math/FixedMath.hpp"
#include "Netcode/NetworkManager.h"
#include "Registry/CommandBuffer.h"
#include "Registry/Registry.h"
#include "Rendering/Gizmos.h"
#include "Rendering/RenderingSystem.h"
#include "Resources/AssetPath.h"
#include "Resources/AssetServer.h"
#include "Resources/Font.h"
#include "Systems/SystemRegistry.h"
#include "UI/TextRenderer.h"
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

    CommandBuffer cmd{};
    SystemRegistry::Get().Init(registry, cmd);
    cmd.Execute(registry);

    AssetServer& as = registry->GetAssetServer();
    as.Load<Font>(AssetPath{ "Data/Fonts/TestFont.casset" });

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

    CommandBuffer cmd{};
    SystemRegistry::Get().Run(registry, cmd, SystemPhase::Simulation);
    cmd.Execute(registry);

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
    CommandBuffer cmd{};

    simTick = tick;
    SystemRegistry::Get().RunSync(registry, cmd, tick, SystemPhase::Simulation);

    cmd.Execute(registry);
}

void World::Render(float alpha)
{
    CommandBuffer cmd{};

    RenderingSystem& rs = RenderingSystem::Get();
    SystemRegistry::Get()
        .Run(registry, cmd, SystemPhase::Presentation);

    cmd.Execute(registry);

    rs.BeginFrame();
    rs.Render3D(registry, alpha);
#if !EDITOR
    rs.RenderUI(registry);
#endif
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

    entt::registry& reg = registry->Get();
    auto testText = reg.create();
    entt::resource<Font> font = registry->GetAssetServer().Load<Font>(AssetPath{ "Data/Fonts/TestFont.casset" });
    reg.emplace<TextRenderer>(testText, font, "Hello World 12345", 75.0f);

    // SceneUtilities::CreateScene(registry, globalCmd, tr1, data);
}

void World::GCPass()
{
    registry->GCPass();
}
