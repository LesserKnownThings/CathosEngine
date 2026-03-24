#include "GridSystem.h"
#include "Map/MapFormat.h"

#include <cstdlib>

void GridSystem::CreateGrid(const MapFormat& mapFormat)
{
    width = mapFormat.width;

    int32_t sectorW = mapFormat.width / SECTOR_SIZE;
    int32_t sectorH = mapFormat.height / SECTOR_SIZE;

    for (int32_t y = 0; y < mapFormat.height; ++y)
    {
        for (int32_t x = 0; x < mapFormat.width; ++x)
        {
            // sectors.emplace_back(Sector
            // {
            //     .intergationField = new IntegrationField(SECTOR_SIZE),
            //     .flowField = new uint8_t(SECTOR_SIZE),
            //     .costType
            // });
        }
    }
}

void GridSystem::FreeGrid()
{
}

MapFormat GridSystem::CreateDebugMapFormat()
{
    MapFormat format{};

    format.width = 160;
    format.height = 320;

    return format;
}