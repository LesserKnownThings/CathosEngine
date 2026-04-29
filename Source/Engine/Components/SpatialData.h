#pragma once

#include <cstdint>
#include <entt/entt.hpp>

struct SpatialData
{
    int32_t cell = -1;
    entt::entity next;
};