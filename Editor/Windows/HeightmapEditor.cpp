#include "HeightmapEditor.h"
#include "Map/MapFormat.h"
#include "Resources/Texture.h"
#include "Utilities/MapUtils.h"
#include "Utilities/TextureImporter.h"
#include <cstdint>
#include <vector>

constexpr float RAD_2_DEG = 180.f / 3.1415926535f;
constexpr float LOW_SLOPE[] = { 0.25f, 0.30f };
constexpr float NORMAL_SLOPE[] = { 0.35f, 0.40f };
constexpr float HIGH_SLOPE[] = { 0.45f, 0.50f };
constexpr float TARGET_HEIGHT = 60.0f;

HeightmapEditor& HeightmapEditor::Get()
{
    static HeightmapEditor instance;
    return instance;
}

void HeightmapEditor::Generate(const std::string& heightmapTexturePath, const std::string& normalTexturePath)
{
    heightmap = heightmapTexturePath;
    normal = normalTexturePath;

    BakeMap();
}

void HeightmapEditor::LoadMap(const std::string& path)
{
    MapFormat map{};
    MapUtils::ImportMap(path, map);
    onMapCreated.raise(map);
}

inline bool IsWalkableEdge(int32_t sectorIndex, int32_t i, PortalDirection direction, bool isA, const MapFormat& map)
{
    const SectorData& s = map.sectors[sectorIndex];

    if (!s.hasCost)
        return true;

    int32_t x, y;

    if (direction == PortalDirection::Vertical)
    {
        y = i;
        x = isA ? (SECTOR_DIM - 1) : 0;
    }
    else
    {
        x = i;
        y = isA ? (SECTOR_DIM - 1) : 0;
    }

    const int32_t index = y * SECTOR_DIM + x;

    const uint8_t cost = s.costBuffer[index];

    return cost != COST_WALL;
}

inline void EmitPortal(int32_t a, int32_t b, int32_t start, int32_t end, PortalDirection dir, MapFormat& map)
{
    Portal p{};
    p.sector = a;
    p.neighbor = b;
    p.start = start;
    p.end = end;
    p.center = (start + end) / 2;
    p.direction = dir;

    map.portals.push_back(p);
}

inline void BuildPortalBetween(int32_t a, int32_t b, PortalDirection direction, MapFormat& map)
{
    int32_t spanStart = -1;

    for (int32_t i = 0; i < SECTOR_DIM; ++i)
    {
        bool walkableA = IsWalkableEdge(a, i, direction, true, map);
        bool walkableB = IsWalkableEdge(b, i, direction, false, map);

        if (walkableA && walkableB)
        {
            if (spanStart == -1)
                spanStart = i;
        }
        else
        {
            if (spanStart != -1)
            {
                EmitPortal(a, b, spanStart, i - 1, direction, map);
                spanStart = -1;
            }
        }
    }

    if (spanStart != -1)
        EmitPortal(a, b, spanStart, SECTOR_DIM - 1, direction, map);
}

void HeightmapEditor::BakeMap()
{
    auto loadNormalFunc = [&](void* normalPixels, const TextureData& data)
    {
        map.width = data.width;
        map.height = data.height;
        uint8_t* pixels = static_cast<uint8_t*>(normalPixels);

        const int wSectors = map.width / SECTOR_DIM;
        const int hSectors = map.height / SECTOR_DIM;
        map.sectors.resize(wSectors * hSectors);

        constexpr int KERNEL_RADIUS = 1;
        const int KERNEL_WIDTH = (KERNEL_RADIUS * 2 + 1);
        const float KERNEL_INV_TOTAL = 1.0f / (KERNEL_WIDTH * KERNEL_WIDTH);

        for (int32_t sy = 0; sy < hSectors; ++sy)
        {
            for (int32_t sx = 0; sx < wSectors; ++sx)
            {
                SectorData& sector = map.sectors[sy * wSectors + sx];
                sector.costBuffer = new uint8_t[SECTOR_SIZE];
                bool hasExpensiveTerrain = false;

                for (int py = 0; py < SECTOR_DIM; ++py)
                {
                    for (int px = 0; px < SECTOR_DIM; ++px)
                    {
                        int globalX = sx * SECTOR_DIM + px;
                        int globalY = sy * SECTOR_DIM + py;

                        if (globalX < KERNEL_RADIUS || globalX >= map.width - KERNEL_RADIUS ||
                            globalY < KERNEL_RADIUS || globalY >= map.height - KERNEL_RADIUS)
                        {
                            sector.costBuffer[py * SECTOR_DIM + px] = COST_WALL;
                            hasExpensiveTerrain = true;
                            continue;
                        }

                        float avg_nx = 0.0f;
                        float avg_ny = 0.0f;
                        float avg_nz = 0.0f;

                        for (int ky = -KERNEL_RADIUS; ky <= KERNEL_RADIUS; ++ky)
                        {
                            for (int kx = -KERNEL_RADIUS; kx <= KERNEL_RADIUS; ++kx)
                            {
                                const int32_t idx = ((globalY + ky) * map.width + (globalX + kx)) * 4;
                                avg_nx += (pixels[idx] / 127.5f) - 1.0f;
                                avg_ny += (pixels[idx + 1] / 127.5f) - 1.0f;
                                avg_nz += (pixels[idx + 2] / 127.5f) - 1.0f;
                            }
                        }
                        avg_nx *= KERNEL_INV_TOTAL;
                        avg_ny *= KERNEL_INV_TOTAL;
                        avg_nz *= KERNEL_INV_TOTAL;

                        if (std::abs(avg_nz) < 0.0001f)
                            avg_nz = 0.0001f;
                        float slopeValue = std::sqrt(avg_nx * avg_nx + avg_ny * avg_ny) / std::abs(avg_nz);

                        uint8_t cost;
                        if (slopeValue < LOW_SLOPE[0])
                            cost = COST_CONSTANT;
                        else if (slopeValue < LOW_SLOPE[1])
                            cost = COST_LOW;
                        else if (slopeValue < NORMAL_SLOPE[1])
                            cost = COST_NORMAL;
                        else if (slopeValue < HIGH_SLOPE[1])
                            cost = COST_HIGH;
                        else
                            cost = COST_WALL;

                        sector.costBuffer[py * SECTOR_DIM + px] = cost;
                        if (cost != COST_CONSTANT)
                            hasExpensiveTerrain = true;
                    }
                }

                if (!hasExpensiveTerrain)
                {
                    delete[] sector.costBuffer;
                    sector.costBuffer = nullptr;
                    sector.hasCost = false;
                }
                else
                {
                    sector.hasCost = true;
                }
            }
        }

        for (int32_t y = 0; y < hSectors; ++y)
        {
            for (int32_t x = 0; x < wSectors; ++x)
            {
                const int32_t sectorA = y * wSectors + x;

                if (x + 1 < wSectors)
                {
                    const int32_t sectorB = y * wSectors + (x + 1);
                    BuildPortalBetween(sectorA, sectorB, PortalDirection::Vertical, map);
                }

                if (y + 1 < hSectors)
                {
                    const int32_t sectorB = (y + 1) * wSectors + x;
                    BuildPortalBetween(sectorA, sectorB, PortalDirection::Horizontal, map);
                }
            }
        }
    };

    TextureImporter::ReadPixels(normal, loadNormalFunc);

    isBaked = true;
    onMapCreated.raise(map);
}