#pragma once

#include <cstdint>
#include <entt/entt.hpp>

// Each cell in the grid
struct SpatialData
{
    int32_t cell = -1;
    entt::entity next;
};