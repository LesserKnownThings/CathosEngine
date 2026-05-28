#include "World.h"

#include "Game/CommandProcessor.h"
#include "Game/Player.h"
#include "Netcode/NetworkManager.h"
#include "Registry/CommandBuffer.h"
#include "Registry/Registry.h"
#include "Rendering/RenderingSystem.h"
#include "Resources/AssetServer.h"
#include "Systems/SystemRegistry.h"
#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <entt/resource/resource.hpp>
#include <glm/ext/vector_float2.hpp>
#include <iostream>

Registry* World::registry = new Registry();
CommandBuffer World::frameStartCommandBuffer{ registry };
CommandBuffer World::frameEndCommandBuffer{ registry };

bool World::Initialize(int argc, const char* argv[])
{
    CommandBuffer cmd{ registry };
    CreateWorld(cmd);
    SystemRegistry::Get().Init(registry, cmd);
    cmd.Execute();

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

    CommandBuffer cmd{ registry };
    SystemRegistry::Get().Run(registry, cmd, SystemPhase::Simulation);
    cmd.Execute();
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
    CommandBuffer cmd{ registry };

    simTick = tick;
    SystemRegistry::Get().RunSync(registry, cmd, tick, SystemPhase::Simulation);

    cmd.Execute();
}

void World::Render(float alpha)
{
    CommandBuffer cmd{ registry };

    RenderingSystem& rs = RenderingSystem::Get();
    SystemRegistry::Get()
        .Run(registry, cmd, SystemPhase::Presentation);

    cmd.Execute();

    rs.BeginFrame();
    rs.Render3D(registry, alpha);
#if !EDITOR
    rs.RenderUI(registry);
#endif
    rs.EndFrame();
}

void World::CreateWorld(CommandBuffer& cmd)
{
    player = new Player(registry);

    AssetServer& as = registry->GetAssetServer();

    entt::registry& reg = registry->Get();
}

void World::Test()
{
    std::cout << "HELLO FROM IMAGE 2" << std::endl;
}

void World::GCPass()
{
    registry->GCPass();
}

void World::FrameStart()
{
    frameStartCommandBuffer.Execute();
}

void World::FrameEnd()
{
    frameEndCommandBuffer.Execute();
}
