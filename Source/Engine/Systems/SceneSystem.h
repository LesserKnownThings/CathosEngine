#pragma once

#include "Components/Transform.h"
#include "Resources/AssetPath.h"
#include "Resources/AssetServer.h"
#include "Resources/MaterialMesh3D.h"
#include "Resources/Model.h"
#include "Resources/Texture.h"
#include "Systems/TransformSystem.h"
#include <cstdint>
#include <entt/entt.hpp>
#include <entt/resource/resource.hpp>
#include <string>

namespace SceneSystem
{
struct SceneData
{
    AssetPath mesh;
    AssetPath material;
    glm::vec4 albedoColor;
    Float3 pos;
    Float3 eulers;
    int32_t count;
};

inline uint64_t GetVec4Hash(const glm::vec4& v)
{
    uint32_t r = static_cast<uint32_t>(v.x);
    uint32_t g = static_cast<uint32_t>(v.y);
    uint32_t b = static_cast<uint32_t>(v.z);
    uint32_t a = static_cast<uint32_t>(v.w);

    uint64_t high = (static_cast<uint64_t>(r) << 32) | g;
    uint64_t low = (static_cast<uint64_t>(b) << 32) | a;

    return high ^ (low >> 1);
}

inline uint64_t CombineHashes(uint64_t h1, uint64_t h2)
{
    h1 ^= h2 + 0x9e3779b97f4a7c15 + (h1 << 6) + (h1 >> 2);
    return h1;
}

// Creates a scene
// A scene is a hierarchy of meshes, they're still separate entities
// This is a utility function, it should not be used to create everyhting, most meshes are single entities so you can create them yourself
inline entt::entity CreateScene(entt::registry& registry, const SceneData& data)
{
    AssetServer& as = registry.ctx().get<AssetServer>();
    auto meshRes = as.Load<Model>(data.mesh);

    const AssetPath baseTexture = data.material.IsValid() ? data.material : AssetPath("Data/Engine/Textures/base_albedo.png");
    auto texture = as.Load<Texture2D>(baseTexture);
    auto matHandle = as.Load<MaterialMesh3DHandle>(baseTexture.GetHash(), texture, data.albedoColor);

    std::vector<entt::entity> parentStack{};

    for (int32_t i = 0; i < meshRes->meshes.size(); ++i)
    {
        const MeshGPUData& mesh = meshRes->meshes[i];

        auto entity = registry.create();
        registry.emplace<LocalTransform>(entity, LocalTransform{
                                                     .rotation = TransformSystem::EulerToQuat(data.eulers),
                                                     .position = data.pos,
                                                 });

        registry.emplace<GlobalTransform>(entity);
        registry.emplace<RenderTransform>(entity);
        registry.emplace<Hierarchy>(entity);
        registry.emplace<Mesh>(entity, Mesh{ meshRes, static_cast<uint8_t>(i) });
        registry.emplace<MaterialMesh3D>(entity, MaterialMesh3D{ matHandle, data.albedoColor });

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