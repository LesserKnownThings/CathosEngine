#pragma once

#include "Math/FixedMath.hpp"
#include <cstdint>
#include <vector>

enum PortalSide : uint8_t
{
    North,
    South,
    East,
    West
};

enum CostType : uint8_t
{
    Empty,
    Wall
};

struct SectorPortal
{
    int32_t sectorA;
    int32_t sectorB;

    int32_t start;
    int32_t end;
    int32_t center;

    PortalSide side;
};

struct SectorData
{
    std::vector<SectorPortal> portals;

    CostType costType;
    uint8_t* costBuffer;
};

struct IntegrationField
{
    FixedT itegratedCost;
    uint8_t integrationFlags;
};

struct Sector
{
    Float3 position;

    IntegrationField* intergationField = nullptr;
    uint8_t* flowField = nullptr;

    SectorData data;
};
