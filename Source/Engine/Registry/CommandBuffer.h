#pragma once

#include "Registry.h"
#include <entt/entt.hpp>
#include <vector>

using Command = std::function<void(entt::registry&)>;

class CommandBuffer
{
  public:
    // The entity creation can be done directly on the registry
    entt::entity Create()
    {
        entt::entity entity = registry->CreateEntity();
        return entity;
    }

    template <typename T>
    void AddComponent(entt::entity e, T component)
    {
        commands.emplace_back(
            [=](entt::registry& reg)
            {
                reg.emplace_or_replace<T>(e, component);
            });
    }

    template <typename T, typename... Args>
    void AddComponent(entt::entity e, Args&&... args)
    {
        commands.emplace_back([=](entt::registry& reg)
                              { reg.emplace_or_replace<T>(e, args...); });
    }

    template <typename T>
    void RemoveComponent(entt::entity e)
    {
        commands.emplace_back([=](entt::registry& reg)
                              { reg.remove<T>(e); });
    }

    template <typename T>
    void ModifyComponent(entt::entity e, T component)
    {
        commands.emplace_back(
            [=](entt::registry& reg)
            {
                reg.get<T>(e) = component;
                reg.patch<T>(e);
            });
    }

    void Destroy(entt::entity e);

  private:
    void Execute()
    {
        entt::registry& reg = registry->Get();

        for (Command& cmd : commands)
        {
            cmd(reg);
        }

        commands.clear();
    }

    CommandBuffer(Registry* inRegistry) : registry(inRegistry) {}

    std::vector<Command> commands;

    Registry* registry;

    friend class World;
    friend class EditorWorld;
};
