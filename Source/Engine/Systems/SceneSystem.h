#pragma once

#include "Components/Transform.h"
#include "Resources/AssetPath.h"
#include "Resources/AssetServer.h"
#include "Resources/Model.h"
#include "Systems/TransformSystem.h"
#include <entt/entt.hpp>

namespace SceneSystem
{
// Creates a scene
// A scene is a hierarchy of meshes, they're still separate entities
inline entt::entity CreateScene(entt::registry& registry, const AssetPath& path, const Float3& pos = {}, const Float3& eulers = {}, bool skipSort = false)
{
    AssetServer& as = registry.ctx().get<AssetServer>();
    auto res = as.Load<Model>(path);

    std::vector<entt::entity> parentStack{};

    for (int32_t i = 0; i < res->meshes.size(); ++i)
    {
        const MeshGPUData& mesh = res->meshes[i];

        auto entity = registry.create();
        registry.emplace<LocalTransform>(entity, LocalTransform{
                                                     .rotation = TransformSystem::EulerToQuat(eulers),
                                                     .position = pos,
                                                 });

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

    // There are instances in which you can skip the sorting, like for example creating 100 units in the same frame
    if (!skipSort)
    {
        auto sortFunc = [](const Mesh& lhs, const Mesh& rhs)
        {
            return lhs.handle->id < rhs.handle->id;
        };
        registry.sort<Mesh>(sortFunc);
    }

    return parentStack[0];
}
} // namespace SceneSystem