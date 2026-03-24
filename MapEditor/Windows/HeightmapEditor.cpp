#include "HeightmapEditor.h"
#include "Map/MapFormat.h"
#include "Resources/SpatialPartition/Sector.h"
#include "Resources/Texture.h"
#include "Utilities/TextureImporter.h"
#include <algorithm>
#include <cstdint>

constexpr float RAD_2_DEG = 180.f / 3.1415926535f;
constexpr float MAX_SLOPE = 45.0f;

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

inline void AddPortal(SectorData& sector, int32_t a, int32_t b, int32_t start, int32_t end, PortalSide side)
{
    SectorPortal p;
    p.sectorA = a;
    p.sectorB = b;
    p.side = side;
    p.start = start;
    p.end = end;
    p.center = ((start + end) / 2);

    sector.portals.push_back(p);
}

inline uint8_t GetCostAtEdge(SectorData& sector, PortalSide side, int32_t index)
{
    if (sector.costType == 0)
        return 0;

    if (index < 0 || index >= SECTOR_DIM)
        return 255;

    switch (side)
    {
    case North:
        return sector.costBuffer[index];

    case South:
        return sector.costBuffer[(SECTOR_DIM - 1) * SECTOR_DIM + index];

    case West:
        return sector.costBuffer[index * SECTOR_DIM];

    case East:
        return sector.costBuffer[index * SECTOR_DIM + (SECTOR_DIM - 1)];

    default:
        return 255;
    }
}

inline PortalSide GetOppositeSide(PortalSide side)
{
    switch (side)
    {
    case PortalSide::East:
        return PortalSide::West;
    case PortalSide::West:
        return PortalSide::East;
    case PortalSide::North:
        return PortalSide::South;
    case PortalSide::South:
        return PortalSide::North;
    }
}

inline void
GeneratePortals(SectorData& sA, uint32_t idA, SectorData& sB, uint32_t idB, PortalSide side)
{
    PortalSide sideB = GetOppositeSide(side);
    int32_t startIdx = -1;

    for (int32_t i = 0; i < SECTOR_DIM; ++i)
    {
        bool walkableA = (sA.costType == 0) || (GetCostAtEdge(sA, side, i) == 0);
        bool walkableB = (sB.costType == 0) || (GetCostAtEdge(sB, sideB, i) == 0);
        bool bothWalkable = walkableA && walkableB;

        if (bothWalkable)
        {
            if (startIdx == -1)
            {
                startIdx = i;
            }
        }
        else
        {
            if (startIdx != -1)
            {
                AddPortal(sA, idA, idB, startIdx, i - 1, side);
                startIdx = -1;
            }
        }
    }

    // Entire side is walkable
    if (startIdx != -1)
    {
        AddPortal(sA, idA, idB, startIdx, SECTOR_DIM - 1, side);
    }
}

inline void CalculateSectorCellElevations(const uint8_t* normalMapBuffer, int32_t startX, int32_t startY, int32_t fullWidth, float outElevations[SECTOR_SIZE], bool& outUnique)
{
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
            if (elevation > MAX_SLOPE)
            {
                outUnique = true;
            }
            outElevations[localIndex] = elevation;
        }
    }
}

void HeightmapEditor::BakeMap()
{
    auto loadNormalFunc = [&](void* normalPixels, const TextureData& normalData)
    {
        // This func is pretty heavy, maybe make it multi thread?
        auto func = [&](void* heightmapPixels, const TextureData& data)
        {
            uint8_t* mPixels = static_cast<uint8_t*>(heightmapPixels);
            const int wSectors = data.width / SECTOR_DIM;
            const int hSectors = data.height / SECTOR_DIM;

            map.width = data.width;
            map.height = data.height;

            const int32_t sectorsCount = wSectors * hSectors;
            map.sectors.resize(sectorsCount);

            for (int32_t sectorY = 0; sectorY < hSectors; ++sectorY)
            {
                for (int32_t sectorX = 0; sectorX < wSectors; ++sectorX)
                {
                    float elevations[SECTOR_SIZE];
                    bool isUnique = false;

                    int32_t startPixelX = sectorX * SECTOR_DIM;
                    int32_t startPixelY = sectorY * SECTOR_DIM;

                    CalculateSectorCellElevations(
                        static_cast<uint8_t*>(normalPixels),
                        startPixelX,
                        startPixelY,
                        data.width,
                        elevations,
                        isUnique);

                    const int32_t sectorIndex = sectorY * wSectors + sectorX;
                    if (isUnique)
                    {
                        map.sectors[sectorIndex].costType = Wall;
                        map.sectors[sectorIndex].costBuffer = new uint8_t[SECTOR_SIZE];

                        for (int32_t i = 0; i < SECTOR_SIZE; ++i)
                        {
                            map.sectors[sectorIndex].costBuffer[i] = elevations[i] < MAX_SLOPE ? 0 : 255;
                        }
                    }
                    else
                    {
                        map.sectors[sectorIndex].costType = Empty;
                        map.sectors[sectorIndex].costBuffer = nullptr;
                    }
                }
            }

            for (int32_t y = 0; y < hSectors; ++y)
            {
                for (int32_t x = 0; x < wSectors; ++x)
                {
                    uint32_t currentID = y * wSectors + x;
                    SectorData& currentSector = map.sectors[currentID];

                    if (x + 1 < wSectors)
                    {
                        uint32_t eastID = y * wSectors + (x + 1);
                        SectorData& eastSector = map.sectors[eastID];
                        GeneratePortals(currentSector, currentID, eastSector, eastID, East);
                    }

                    if (y + 1 < hSectors)
                    {
                        uint32_t southID = (y + 1) * wSectors + x;
                        SectorData& southSector = map.sectors[southID];
                        GeneratePortals(currentSector, currentID, southSector, southID, South);
                    }
                }
            }
        };

        TextureImporter::ReadPixels(heightmap, func);
    };

    TextureImporter::ReadPixels(normal, loadNormalFunc);

    onMapCreated.raise(map);
}