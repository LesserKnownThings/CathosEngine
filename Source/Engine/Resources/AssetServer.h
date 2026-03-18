#pragma once

#include "Resources/AssetPath.h"
#include "Resources/ResourceCache.h"
#include <cstdint>
#include <entt/core/fwd.hpp>
#include <entt/core/type_info.hpp>
#include <entt/resource/cache.hpp>
#include <entt/resource/resource.hpp>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename T>
struct AssetLoader
{
    using result_type = std::shared_ptr<T>;

    template <typename... Args>
    result_type operator()(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
};

template <typename T>
struct ResourceCacheProxy : public IResourceCache
{
  public:
    entt::resource_cache<T, AssetLoader<T>> cache;

    void PurgeUnused() override
    {
        std::vector<uint64_t> toRemove;

        for (auto it = cache.begin(); it != cache.end(); ++it)
        {
            toRemove.push_back(it->first);
        }

        for (uint64_t id : toRemove)
        {
            cache.erase(id);
        }
    }

    uint32_t Count() const override { return cache.size(); }
};

class AssetServer
{
  public:
    AssetServer() = default;
    AssetServer(const AssetServer&) = delete;
    AssetServer& operator=(const AssetServer&) = delete;

    template <typename T, typename... Args>
    entt::resource<T> Load(const AssetPath& path, Args... args)
    {
        ResourceCacheProxy<T>& proxy = GetOrCreateCache<T>(entt::type_id<T>().index());

        auto result = proxy.cache.load(
            path.GetHash(),
            path,
            std::forward<args>(args)...);

        return result.first->second;
    }

    // This one requires you create your own hash like "my_asset"_hs
    template <typename T, typename... Args>
    entt::resource<T> Load(uint64_t id, Args... args)
    {
        ResourceCacheProxy<T>& proxy = GetOrCreateCache<T>(entt::type_id<T>().index());
        auto result = proxy.cache.load(
            id,
            std::forward<Args>(args)...);

        return result.first->second;
    }

    void GCPass()
    {
        for (auto& [path, ptr] : cache)
        {
            ptr->PurgeUnused();
        }
    }

  private:
    template <typename T>
    ResourceCacheProxy<T>& GetOrCreateCache(uint32_t id)
    {
        if (cache.find(id) == cache.end())
        {
            cache.emplace(id, std::move(std::make_unique<ResourceCacheProxy<T>>()));
        }

        return static_cast<ResourceCacheProxy<T>&>(*cache[id]);
    }

    std::unordered_map<uint32_t, std::unique_ptr<IResourceCache>> cache;
};