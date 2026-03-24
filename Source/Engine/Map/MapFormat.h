#pragma once

#include "Resources/SpatialPartition/Sector.h"

#include <cstdint>
#include <vector>

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

constexpr int32_t SECTOR_DIM = 10;
constexpr int32_t SECTOR_SIZE = SECTOR_DIM * SECTOR_DIM;

constexpr uint8_t CLEAR_CELL = 1;
constexpr uint8_t WALL_CELL = 255;

struct MapFormat
{
    // Physical size of the map, this will be split into sectors (160x160 is the smallest map size 16x16 sectors)
    int32_t width;  // multiple of 160
    int32_t height; // multiple of 160

    std::vector<SectorData> sectors;
};
