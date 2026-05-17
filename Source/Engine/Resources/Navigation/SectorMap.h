#pragma once

#include "Map/MapFormat.h"
#include "Math/FixedMath.hpp"
#include <array>
#include <cstdint>

struct IntegrationField
{
    FixedT itegratedCost;
    uint8_t integrationFlags;
};

struct Sector
{
    std::array<IntegrationField, SECTOR_SIZE> integrationField;
    std::array<uint8_t, SECTOR_SIZE> flowField;
    int32_t dataIndex;

    Sector() noexcept
    {
        dataIndex = -1; // INVALID DATA
    }
};

struct SectorMap
{
    std::vector<Sector> sectors;
};
