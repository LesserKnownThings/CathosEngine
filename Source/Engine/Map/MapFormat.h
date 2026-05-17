#pragma once

#include "Math/FixedMath.hpp"
#include <cstdint>
#include <vector>

constexpr int32_t SECTOR_DIM = 10;
constexpr int32_t SECTOR_SIZE = SECTOR_DIM * SECTOR_DIM;

constexpr uint8_t CLEAR_CELL = 1;
constexpr uint8_t WALL_CELL = 255;

constexpr uint8_t COST_CONSTANT = 0;
constexpr uint8_t COST_NORMAL = 1;
constexpr uint8_t COST_LOW = 2;
constexpr uint8_t COST_HIGH = 3;
constexpr uint8_t COST_WALL = 255;

enum class PortalDirection : uint8_t
{
    Vertical,
    Horizontal,
};

struct Portal
{
    int32_t sector;
    int32_t neighbor;

    int32_t start;
    int32_t end;
    int32_t center;

    PortalDirection direction;
};

struct SectorData
{
    bool hasCost;
    uint8_t* costBuffer;
};

/**
Cathos map format (.cmf) is the format used to generate the map for the game
using a heightmap image.

Unlike other engines, Cathos stores navigation data in the map format. So when
a map is generated it will automatically generate the map grid and navigation
data.

The map is as follows
Cell 1x1
Sector 10x10 cells
The grid has multiple sectors
I'm doing the same implementation as this article by Emerson Elijah:
https://www.gameaipro.com/GameAIPro/GameAIPro_Chapter23_Crowd_Pathfinding_and_Steering_Using_Flow_Field_Tiles.pdf

The sector cost is:
0 -> empty
1 - 254 -> traversing path cost
255 -> wall

*/
struct MapFormat
{
    // Physical size of the map, this will be split into sectors (160x160 is the smallest map size 16x16 sectors)
    int32_t width;  // multiple of 160
    int32_t height; // multiple of 160

    std::vector<SectorData> sectors;
    std::vector<Portal> portals;

    int32_t HorizontalSectors() const { return width / SECTOR_DIM; }
    int32_t VerticalSectors() const { return height / SECTOR_DIM; }

    int32_t GetSectorIndex(const Float3& position) const
    {
        return static_cast<int32_t>(position.x) * HorizontalSectors() + static_cast<int32_t>(position.x);
    }

    Float3 GetPortalCenter(int32_t index) const
    {
        return GetPortalPos(index, portals[index].center);
    }

    Float3 GetPortalPos(int32_t portalIndex, int32_t slotIndex) const
    {
        const Portal& portal = portals[portalIndex];
        const int32_t x = slotIndex % SECTOR_DIM + portal.sector * SECTOR_DIM;
        const int32_t z = slotIndex / SECTOR_DIM + portal.sector * SECTOR_DIM;
        return Float3{ x, 0.0f, z };
    }
};
