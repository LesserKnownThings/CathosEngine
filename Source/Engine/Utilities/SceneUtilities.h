#pragma once

#include "Components/Collider.h"
#include "Components/Transform.h"
#include "Components/Visibility.h"
#include "Math/TransformMath.hpp"
#include "Registry/CommandBuffer.h"
#include "Registry/Registry.h"
#include "Resources/AssetPath.h"
#include "Resources/AssetServer.h"
#include "Resources/MaterialMesh3D.h"
#include "Resources/Model.h"
#include "Resources/Texture.h"
#include <cstdint>
#include <entt/entt.hpp>
#include <entt/resource/resource.hpp>
#include <string>

struct SceneData
{
    AssetPath mesh;
    AssetPath material;
    glm::vec4 albedoColor;
    bool isStatic;
    Collider collider;
};

namespace SceneUtilities
{
// Creates a scene
// A scene is a hierarchy of meshes, they're still separate entities
// This is a utility function, it should not be used to create everyhting, most meshes are single entities so you can create them yourself
inline void CreateScene(Registry* registry, CommandBuffer& cmd, const Transform transform, const SceneData& data)
{
    entt::registry& reg = registry->Get();

    AssetServer& as = reg.ctx().get<AssetServer>();
    auto meshRes = as.Load<Model>(data.mesh);

    const AssetPath baseTexture = data.material.IsValid() ? data.material : AssetPath("Data/Engine/Textures/base_albedo.png");
    auto texture = as.Load<Texture2D>(baseTexture);
    auto matHandle = as.Load<MaterialMesh3DHandle>(baseTexture.GetHash(), texture, data.albedoColor);

    std::vector<uint32_t> parentStack{};

    for (int32_t i = 0; i < meshRes->meshes.size(); ++i)
    {
        const MeshGPUData& mesh = meshRes->meshes[i];

        uint32_t entity = cmd.CreateEntity();
        cmd.AddComponent<LocalTransform>(entity, LocalTransform{
                                                     .rotation = TransformMath::EulerToQuat(transform.eulers),
                                                     .position = transform.position,
                                                     .scale = transform.scale,
                                                 });
        cmd.AddComponent(entity, GlobalTransform{});
        cmd.AddComponent(entity, RenderTransform{});
        cmd.AddComponent(entity, Hierarchy{});
        cmd.AddComponent(entity, Mesh{ meshRes, static_cast<uint8_t>(i) });
        cmd.AddComponent(entity, MaterialMesh3D{ matHandle, data.albedoColor });
        cmd.AddComponent(entity, Visible{});

        // Only the parent should have a collider
        if (i == 0)
            cmd.AddComponent(entity, data.collider);

        if (mesh.depth >= parentStack.size())
        {
            parentStack.resize(mesh.depth + 1, INVALID_ENTITY);
        }

        parentStack[mesh.depth] = entity;

        if (mesh.depth > 0)
        {
            uint32_t parent = parentStack[mesh.depth - 1];

            if (parent != INVALID_ENTITY)
            {
                cmd.Link(entity, parent);
            }
        }
    }
}
} // namespace SceneUtilities