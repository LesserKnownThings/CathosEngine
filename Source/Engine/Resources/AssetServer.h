#pragma once

#include "Resources/AssetPath.h"
#include "Resources/ResourceCache.h"
#include <cstdint>
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

    result_type operator()(const AssetPath& path)
    {
        return std::make_shared<T>(path);
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

    template <typename T>
    entt::resource<T> Load(const AssetPath& path)
    {
        ResourceCacheProxy<T>& proxy = GetOrCreateCache<T>(path);

        auto result = proxy.cache.load(path.GetHash(), path);

        return result.first->second;
    }

    void GCPass()
    {
        for (auto& [path, ptr] : caches)
        {
            ptr->PurgeUnused();
        }
    }

  private:
    template <typename T>
    ResourceCacheProxy<T>& GetOrCreateCache(const AssetPath& path)
    {
        if (caches.find(path) == caches.end())
        {
            caches.emplace(path, std::move(std::make_unique<ResourceCacheProxy<T>>()));
        }

        return static_cast<ResourceCacheProxy<T>&>(*caches[path]);
    }

    std::unordered_map<AssetPath, std::unique_ptr<IResourceCache>> caches;
};