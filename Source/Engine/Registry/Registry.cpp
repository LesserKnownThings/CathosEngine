#include "Registry.h"

#include "Components/Transform.h"
#include "Resources/AssetServer.h"
#include <entt/entity/fwd.hpp>

inline void CleanupHierarchy(entt::registry& registry, entt::entity parent)
{
    const auto& hierarchy = registry.get<Hierarchy>(parent);

    entt::entity current = hierarchy.firstChild;
    while (current != entt::null)
    {
        entt::entity next = registry.get<Hierarchy>(current).nextSibling;
        registry.destroy(current);

        current = next;
    }
}

Registry::~Registry()
{
}

Registry::Registry()
{
    registry.ctx().emplace<AssetServer>();
    registry.on_destroy<Hierarchy>().connect<&CleanupHierarchy>();
}

void Registry::GCPass()
{
    AssetServer& as = registry.ctx().get<AssetServer>();
    as.GCPass();
}

AssetServer& Registry::GetAssetServer()
{
    return registry.ctx().get<AssetServer>();
}
