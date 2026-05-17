#pragma once

#include "Rendering/VkData.h"
#include <array>
#include <cstdint>
#include <glm/glm.hpp>

struct QuadVertex
{
    glm::vec2 position;
    glm::vec2 uv;
};

static constexpr std::array<QuadVertex, 4> QuadVertices = { {
    { { 0.0f, 0.0f }, { 0.0f, 1.0f } },
    { { 1.0f, 0.0f }, { 1.0f, 1.0f } },
    { { 1.0f, 1.0f }, { 1.0f, 0.0f } },
    { { 0.0f, 1.0f }, { 0.0f, 0.0f } },
} };

static constexpr std::array<uint32_t, 6> QuadIndices = { { 0, 1, 2,
                                                           2, 3, 0 } };

struct UIMeshData
{
    uint32_t verticesCount = 0;
    uint32_t indicesCount = 0;

    std::vector<QuadVertex> vertices;
    std::vector<uint32_t> indices;
};

struct UIMeshGPUData
{
    AllocatedBuffer vertices;
    AllocatedBuffer indices;

    uint32_t verticesCount;
    uint32_t indicesCount;
};