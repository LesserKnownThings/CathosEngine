#include "ModelImporter.h"

#include "Debug/DebugSystem.h"
#include "Resources/Model.h"
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <format>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <unordered_map>
#include <vector>

constexpr std::string MODEL_IMPORTER_LOG = "ModelImporter";

namespace AssimpGLMHelpers
{
inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
{
    glm::mat4 to;
    // the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1;
    to[1][0] = from.a2;
    to[2][0] = from.a3;
    to[3][0] = from.a4;
    to[0][1] = from.b1;
    to[1][1] = from.b2;
    to[2][1] = from.b3;
    to[3][1] = from.b4;
    to[0][2] = from.c1;
    to[1][2] = from.c2;
    to[2][2] = from.c3;
    to[3][2] = from.c4;
    to[0][3] = from.d1;
    to[1][3] = from.d2;
    to[2][3] = from.d3;
    to[3][3] = from.d4;
    return to;
}

inline glm::vec3 GetGLMVec(const aiVector3D& vec)
{
    return glm::vec3(vec.x, vec.y, vec.z);
}

inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
{
    return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
}
} // namespace AssimpGLMHelpers

inline void ProcessMesh(aiMesh* mesh, MeshData& outMeshData)
{
    outMeshData.vertices.reserve(mesh->mNumVertices);

    for (int32_t i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex{};
        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        vertex.tangent.x = mesh->mTangents[i].x;
        vertex.tangent.y = mesh->mTangents[i].y;
        vertex.tangent.z = mesh->mTangents[i].z;

        vertex.bitangent.x = mesh->mBitangents[i].x;
        vertex.bitangent.y = mesh->mBitangents[i].y;
        vertex.bitangent.z = mesh->mBitangents[i].z;

        if (mesh->HasNormals())
        {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        if (mesh->mTextureCoords[0])
        {
            // TODO will need to test this with different models
            vertex.uv.x = mesh->mTextureCoords[0][i].x;
            vertex.uv.y = mesh->mTextureCoords[0][i].y;
        }
        else
            vertex.uv = glm::vec2(0.0f, 0.0f);

        outMeshData.vertices.push_back(vertex);
    }

    for (int32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace face = mesh->mFaces[i];

        for (int32_t j = 0; j < face.mNumIndices; ++j)
        {
            outMeshData.indices.push_back(face.mIndices[j]);
            outMeshData.indicesCount++;
        }
    }

    // Building skeleton
    std::unordered_map<const char*, uint32_t> boneMap{};
    uint32_t boneCount = 0;

    for (int32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        int32_t boneID = -1;
        const char* boneName = mesh->mBones[boneIndex]->mName.C_Str();

        auto it = boneMap.find(boneName);
        if (it == boneMap.end())
        {
            boneID = boneCount++;
            boneMap.emplace(boneName, boneID);
        }
        else
        {
            boneID = it->second;
        }

        const aiVertexWeight* weights = mesh->mBones[boneIndex]->mWeights;
        int32_t nWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int32_t weightIndex = 0; weightIndex < nWeights; ++weightIndex)
        {
            int32_t vertexID = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            for (int32_t i = 0; i < MAX_BONE_INFLUENCE; ++i)
            {
                if (outMeshData.vertices[vertexID].boneIDs[i] < 0)
                {
                    outMeshData.vertices[vertexID].boneIDs[i] = boneID;
                    outMeshData.vertices[vertexID].weights[i] = weight;
                    break;
                }
            }
        }
    }
}

inline void ProcessNodeForModel(aiNode* rootNode, const aiScene* scene, std::vector<MeshData>& outMeshes, uint32_t depth)
{
    for (int32_t i = 0; i < rootNode->mNumMeshes; ++i)
    {
        MeshData meshData{};
        meshData.depth = depth++;

        aiMesh* mesh = scene->mMeshes[rootNode->mMeshes[i]];
        meshData.verticesCount = mesh->mNumVertices;

        ProcessMesh(mesh, meshData);

        outMeshes.push_back(meshData);
    }

    for (int32_t i = 0; i < rootNode->mNumChildren; ++i)
    {
        ProcessNodeForModel(rootNode->mChildren[i], scene, outMeshes, depth);
    }
}

std::vector<MeshData> ModelImporter::ImportModel(const std::string& path)
{
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path,
                                           aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_ConvertToLeftHanded);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        LOG(MODEL_IMPORTER_LOG, Warning, std::format("Error loading model: [{}]", import.GetErrorString()));
        return {};
    }

    std::vector<MeshData> outMeshes{};
    ProcessNodeForModel(scene->mRootNode, scene, outMeshes, 0);
    return outMeshes;
}