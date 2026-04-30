#include "HeightmapEditor.h"
#include "Map/MapFormat.h"
#include "Resources/SpatialPartition/Sector.h"
#include "Resources/Texture.h"
#include "Utilities/MapUtils.h"
#include "Utilities/TextureImporter.h"
#include <algorithm>
#include <cstdint>
#include <vector>

constexpr float RAD_2_DEG = 180.f / 3.1415926535f;
constexpr float LOW_SLOPE[] = { 0.45f, 0.50f };
constexpr float NORMAL_SLOPE[] = { 0.55f, 0.60f };
constexpr float HIGH_SLOPE[] = { 0.65f, 0.70f };
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

inline bool CalculateSectorCellElevations(const uint8_t* normalMapBuffer, int32_t startX, int32_t startY, int32_t fullWidth, float outElevations[SECTOR_SIZE])
{
    bool isUnique = false;

    for (int32_t y = 0; y < SECTOR_DIM; ++y)
    {
        for (int32_t x = 0; x < SECTOR_DIM; ++x)
        {
            int32_t gX = startX + x;
            int32_t gY = startY + y;

            const int32_t pixel = (gY * fullWidth + gX) * 4;
            const int32_t localIndex = y * SECTOR_DIM + x;

            uint8_t bVal = normalMapBuffer[pixel + 2];
            float nz = (bVal / 255.0f) * 2.0f - 1.0f;

            nz = std::max(-1.0f, std::min(1.0f, nz));
            float angleRad = std::acos(nz);

            const float elevation = angleRad * RAD_2_DEG;
            if (elevation >= LOW_SLOPE[0])
            {
                isUnique = true;
            }
            outElevations[localIndex] = elevation;
        }
    }

    return isUnique;
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

                        // --- Average the Local Neighborhood ---
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
    };

    TextureImporter::ReadPixels(normal, loadNormalFunc);

    isBaked = true;
    onMapCreated.raise(map);
}