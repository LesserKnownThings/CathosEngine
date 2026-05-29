#include "MessageSystem.h"
#include "Engine.h"
#include "LUA/Message.h"
#include "Registry/CommandBuffer.h"
#include "Registry/Registry.h"
#include "Systems/SystemRegistry.h"

REGISTER_SYSTEM(MessageSystem, SystemPhase::Simulation, DEPENDENCIES({}), DEPENDENCIES({}), 1000)

void MessageSystem::Init(Registry* registry, CommandBuffer& cmd)
{
}

void MessageSystem::Run(Registry* registry, CommandBuffer& cmd)
{
    auto processMessages = [&](entt::entity entity, const UIMessage& message)
    {
        switch (message.id)
        {
        case UIMessages::CloseGame:
            Engine::Get()->CloseGame();
            break;
        }

        cmd.Destroy(entity);
    };
    registry->Get().view<UIMessage>().each(processMessages);
}

void MessageSystem::RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick)
{
    auto processMessages = [&](entt::entity entity, const GameMessage& message)
    {
        switch (message.id)
        {
        }

        cmd.Destroy(entity);
    };
    registry->Get().view<GameMessage>().each(processMessages);
}