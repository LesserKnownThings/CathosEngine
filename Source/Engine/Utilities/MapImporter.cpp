#include "MapImporter.h"

#include "Debug/DebugSystem.h"
#include "Map/MapFormat.h"
#include "Map/SpatialPartition/GridSystem.h"
#include "Resources/AssetPath.h"

#include <cstdint>
#include <format>
#include <fstream>

constexpr std::string IMPORTER_LOG = "MapImporter";

bool MapImporter::ImportMap(const AssetPath& path, MapFormat& outMap)
{
    std::ifstream file(path.GetPath(), std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        LOG(IMPORTER_LOG, Error, std::format("Failed to open file: [{}]", path.GetPath()));
        return false;
    }

    file.read(reinterpret_cast<char*>(&outMap.width), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&outMap.height), sizeof(int32_t));

    const int32_t widthSectors = outMap.width / SECTOR_SIZE;
    const int32_t heightSectors = outMap.height / SECTOR_SIZE;
    const int32_t totalSectors = widthSectors * heightSectors;

    // outMap.sectors.resize(totalSectors);
    // for (int32_t i = 0; i < totalSectors; ++i)
    // {
    //     SectorStaticData& data = outMap.sectors[i];
    //     uint8_t& costType = data.costType;

    //     file.read(reinterpret_cast<char*>(&data.portalsCount), sizeof(int32_t));
    //     file.read(reinterpret_cast<char*>(data.portals), sizeof(SectorPortal) * data.portalsCount);
    //     file.read(reinterpret_cast<char*>(&costType), 1);

    //     if (costType == 1)
    //     {
    //         data.costBuffer = new uint8_t(SECTOR_SIZE);
    //         file.read(reinterpret_cast<char*>(data.costBuffer), SECTOR_SIZE);
    //     }
    // }

    // Portal creation

    file.close();

    return true;
}