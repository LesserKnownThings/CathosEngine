#pragma once

#include "Math/FixedMath.hpp"
#include <cstdint>

enum CostType : uint8_t
{
    Empty,
    Unique
};

struct SectorData
{
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
