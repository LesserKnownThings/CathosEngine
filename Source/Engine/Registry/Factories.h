#pragma once

#include "Debug/DebugSystem.h"
#include <cstdint>
#include <entt/entt.hpp>
#include <type_traits>

constexpr std::string FACTORIES_LOG = "Factories";

class Factories
{
  public:
    Factories();

    static void EmplaceByType(entt::registry& reg, entt::entity e, uint32_t typeId, const uint8_t* data)
    {
        auto it = componentFactory.find(typeId);
        if (it != componentFactory.end())
        {
            it->second(reg, e, data);
        }
        else
        {
            LOG(FACTORIES_LOG, Error, "Missing component factory!");
        }
    }

  private:
    using EmplaceFn = void (*)(entt::registry&, entt::entity, const uint8_t*);
    static std::unordered_map<uint32_t, EmplaceFn> componentFactory;

    template <typename T>
    void RegisterComponentType()
    {
        componentFactory[entt::type_id<T>().hash()] = [](entt::registry& reg, entt::entity e, const uint8_t* data)
        {
            if constexpr (std::is_empty_v<T>)
            {
                reg.emplace_or_replace<T>(e);
            }
            else
            {
                const T& component = *reinterpret_cast<const T*>(data);
                reg.emplace_or_replace<T>(e, component);
            }
        };
    }
};