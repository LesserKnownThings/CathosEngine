#pragma once

#include <entt/entt.hpp>

struct AssetServer;

class Registry
{
  public:
    ~Registry();
    Registry();

    void GCPass();

    entt::entity CreateEntity()
    {
        return registry.create();
    }

    template <typename T>
    T& Get(entt::entity entity)
    {
        return registry.get<T>(entity);
    }

    template <typename T>
    const T& Get(entt::entity entity) const
    {
        return registry.get<T>(entity);
    }

    template <typename T>
    T& AddResource(const T& res)
    {
        return registry.ctx().emplace<T>(res);
    }

    template <typename T, typename... Args>
    T& AddResource(Args&&... args)
    {
        return registry.ctx().emplace<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    T& GetResource()
    {
        return registry.ctx().get<T>();
    }

    template <typename T>
    const T& GetResource() const
    {
        return registry.ctx().get<T>();
    }

    template <typename T>
    bool ContainsResource() const
    {
        return registry.ctx().contains<T>();
    }

    AssetServer& GetAssetServer();
    entt::registry& Get() { return registry; }

  private:
    entt::registry registry{};
};
