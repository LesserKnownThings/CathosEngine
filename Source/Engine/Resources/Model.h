#pragma once

#include "Rendering/RenderingSystem.h"
#include "Resources/AssetPath.h"
#include "Utilities/ModelImporter.h"
#include <algorithm>
#include <cstdint>
#include <entt/resource/resource.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/glm.hpp>
#include <vector>

constexpr uint32_t MAX_BONE_INFLUENCE = 4;

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 uv;

    glm::ivec4 boneIDs = glm::ivec4{ -1 };
    glm::vec4 weights = glm::vec4(0.0f);
};

struct InstanceData
{
    glm::vec2 position;
    glm::vec2 scale;
    uint32_t textureIndex;
};

struct MeshData
{
    // Depth is used for hierarchy transform
    uint32_t depth = 0;
    uint32_t verticesCount = 0;
    uint32_t indicesCount = 0;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct MeshGPUData
{
    AllocatedBuffer vertices;
    AllocatedBuffer indices;

    uint32_t verticesCount;
    uint32_t indicesCount;
    int32_t depth; // Used for hierarchy
};

// This will be used to build the ECS meshes in the world
struct Model
{
    Model(const AssetPath& path)
        : id(path.GetHash())
    {
        std::vector<MeshData> mData = ModelImporter::ImportModel(path.GetPath());

        RenderingSystem& rs = RenderingSystem::Get();

        std::sort(mData.begin(), mData.end(), [](const MeshData& a, const MeshData& b)
                  { return a.depth < b.depth; });

        for (const MeshData& meshData : mData)
        {
            meshes.push_back(rs.CreateMesh(meshData));
        }
    }

    ~Model()
    {
        RenderingSystem& rs = RenderingSystem::Get();
        for (const MeshGPUData& mesh : meshes)
        {
            rs.DestroyMesh(mesh);
        }
    }

    std::vector<MeshGPUData> meshes;
    uint64_t id;
};

struct Mesh
{
    entt::resource<Model> handle;
    uint8_t meshIndex;

    
};
