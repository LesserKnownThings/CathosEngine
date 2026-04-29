#pragma once

#include "Map/MapFormat.h"
#include "Resources/SpatialPartition/Sector.h"
#include <atomic>
#include <entt/entt.hpp>
#include <vector>

struct AtomicGrid
{
    int32_t width;
    int32_t height;

    std::vector<std::atomic<entt::entity>> heads;
    std::vector<SectorData> sectors;

    AtomicGrid(MapFormat&& map)
        : width(map.width),
          height(map.height),
          heads(map.width * map.height),
          sectors(std::move(map.sectors))
    {
    }

    AtomicGrid(const AtomicGrid&) = delete;
    AtomicGrid(AtomicGrid&&) = delete;
};
