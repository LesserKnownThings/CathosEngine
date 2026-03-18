#pragma once

#include "Math/FixedMath.hpp"
#include <cstdint>
#include <vector>

/**
 * The grid is as follows
 * Cell 1x1
 * Chunk 10x10 cells
 * The grid has multiple sectors
 * I'm doing the same implementation as this article by Emerson Elijah:
 * https://www.gameaipro.com/GameAIPro/GameAIPro_Chapter23_Crowd_Pathfinding_and_Steering_Using_Flow_Field_Tiles.pdf
 */

constexpr int32_t CHUNK_SIZE = 100; // 10x10
constexpr uint8_t EMPTY_CELL = 0;
constexpr uint8_t WALL_CELL = 255;

enum PortalSide : uint8_t
{
    North,
    South,
    East,
    West
};

struct IntegrationField
{
    FixedT itegratedCost;
    uint8_t integrationFlags;
};

struct Sector
{
    IntegrationField* intergationField = nullptr;
    uint8_t* flowField = nullptr;
    uint8_t* costField = nullptr;
};

struct SectorPortal
{
    uint32_t sectorA;
    uint32_t sectorB;

    uint32_t start;
    uint32_t end;
    uint32_t center;

    PortalSide side;
};

class GridSystem
{
  public:
    void CreateGrid(int32_t size);
    void FreeGrid();

    int32_t GetGridSize() const { return gridSize; }

    static const std::vector<uint8_t> CLEAR_COST;

  private:
    uint8_t* flowField = nullptr;
    /*
     *   0 -> empty
     *   1 - 254 -> traversing path cost
     *   255 -> wall
     */
    uint8_t* costField = nullptr;
    IntegrationField* integrationField = nullptr;

    std::vector<SectorPortal*   > portals;

    int32_t gridSize;
};