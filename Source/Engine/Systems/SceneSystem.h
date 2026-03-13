#pragma once

#include "Components/Transform.h"
#include "Resources/AssetPath.h"
#include "Resources/AssetServer.h"
#include "Resources/Model.h"
#include <entt/entt.hpp>

namespace SceneSystem
{
// Creates a scene
// A scene is a hierarchy of meshes, they're still separate entities
inline entt::entity CreateScene(entt::registry& registry, const AssetPath& path)
{
    AssetServer& as = registry.ctx().get<AssetServer>();
    auto res = as.Load<Model>(path);

    std::vector<entt::entity> parentStack{};

    for (int32_t i = 0; i < res->meshes.size(); ++i)
    {
        const MeshGPUData& mesh = res->meshes[i];

        auto entity = registry.create();
        registry.emplace<LocalTransform>(entity);
        registry.emplace<GlobalTransform>(entity);
        registry.emplace<RenderTransform>(entity);
        registry.emplace<Hierarchy>(entity);
        registry.emplace<Mesh>(entity, Mesh{ res, static_cast<uint8_t>(i) });

        if (mesh.depth >= parentStack.size())
        {
            parentStack.resize(mesh.depth + 1, entt::null);
        }

        parentStack[mesh.depth] = entity;

        if (mesh.depth > 0)
        {
            entt::entity parent = parentStack[mesh.depth - 1];

            if (parent != entt::null)
            {
                Hierarchy::LinkEntities(registry, entity, parent);
            }
        }
    }

    return parentStack[0];
}
} // namespace SceneSystem