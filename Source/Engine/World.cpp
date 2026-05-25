#include "World.h"

#include "Game/CommandProcessor.h"
#include "Game/Player.h"
#include "Netcode/NetworkManager.h"
#include "Registry/CommandBuffer.h"
#include "Registry/Registry.h"
#include "Rendering/RenderingSystem.h"
#include "Resources/AssetPath.h"
#include "Resources/AssetServer.h"
#include "Resources/Font.h"
#include "Resources/Texture.h"
#include "Systems/SystemRegistry.h"
#include "UI/Button.h"
#include "UI/LayoutBox.h"
#include "UI/NineSlice.h"
#include "UI/TextRenderer.h"
#include "UI/UIMaterial.h"
#include "UI/UITransform.h"
#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <entt/resource/resource.hpp>
#include <glm/ext/vector_float2.hpp>
#include <iostream>

bool World::Initialize(int argc, const char* argv[])
{
    registry = new Registry();

    CommandBuffer cmd{};
    CreateWorld(cmd);
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

    CommandBuffer cmd{};
    SystemRegistry::Get().Run(registry, cmd, SystemPhase::Simulation);
    cmd.Execute(registry);
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

void World::CreateWorld(CommandBuffer& cmd)
{
    registry = new Registry();
    player = new Player(registry);

    entt::registry& reg = registry->Get();

    auto boxEntity = reg.create();
    VBox box{
        .offset = glm::vec4(0.0f),
        .spacing = 15.0f,
        .childStart = ChildStart::Start,
        .controlHSize = true,
        .controlVSize = true,
    };
    UITransform boxTransform{
        .position = glm::vec2(100.0f),
        .size = glm::vec2(100.0f),
        .localZOrder = 0,
    };
    reg.emplace<UIPivot>(boxEntity);
    reg.emplace<UIAnchor>(boxEntity, glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    reg.emplace<VBox>(boxEntity, box);
    reg.emplace<UITransform>(boxEntity, boxTransform);

    entt::resource<Texture2D> testImageTexture = registry->GetAssetServer().Load<Texture2D>(AssetPath{ "Data/Images/cover.png" });

    const int32_t childrenSize = 0;
    std::vector<entt::entity> children(childrenSize);
    for (int32_t i = 0; i < childrenSize; ++i)
    {
        auto imageEntity = reg.create();
        reg.emplace<UIMaterial>(imageEntity, testImageTexture);
        reg.emplace<UITransform>(imageEntity, UITransform{ .size = glm::vec2(0.0f, 0.f) });
        reg.emplace<ChildOf>(imageEntity, boxEntity);
        reg.emplace<NineSlice>(imageEntity, 60.0f, 60.0f, 60.0f, 60.0f);

        children[i] = imageEntity;
    }

    reg.emplace<Children>(boxEntity, children);

    entt::resource<Texture2D> baseAbledo = registry->GetAssetServer().Load<Texture2D>(AssetPath{ "Data/Engine/Textures/base_albedo.png" });

    for (int32_t i = 0; i < 2; ++i)
    {
        auto testImage2 = reg.create();
        reg.emplace<UIMaterial>(testImage2, testImageTexture);
        UITransform testImageTr2{
            .position = glm::vec2(0.0f + i * 100.0f),
            .size = glm::vec2(500.0f),
            .localZOrder = 10,
        };
        reg.emplace<UITransform>(testImage2, testImageTr2);
        reg.emplace<UIEventStyle>(testImage2, glm::vec4(1.0f), glm::vec4(1.0f, 1.0f, 0.0f, 0.2f), glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));

        Button button{};
        button.Connect<&World::Test>(this);
        reg.emplace<Button>(testImage2, button);
    }
}

void World::Test()
{
    std::cout << "HELLO FROM IMAGE 2" << std::endl;
}

void World::GCPass()
{
    registry->GCPass();
}
