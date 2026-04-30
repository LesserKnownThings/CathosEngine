#pragma once

#include "Math/FixedMath.hpp"
#include <cstdint>

constexpr uint8_t COST_CONSTANT = 0;
constexpr uint8_t COST_NORMAL = 1;
constexpr uint8_t COST_LOW = 2;
constexpr uint8_t COST_HIGH = 3;
constexpr uint8_t COST_WALL = 255;

struct SectorData
{
    bool hasCost;
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
