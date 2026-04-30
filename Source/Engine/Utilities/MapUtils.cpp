#include "MapUtils.h"

#include "Debug/DebugSystem.h"
#include "Map/MapFormat.h"
#include "Resources/SpatialPartition/Sector.h"

#include <cstdint>
#include <format>
#include <fstream>

constexpr std::string MAP_UTILS_LOG = "MapUtils";

bool MapUtils::ImportMap(const std::string& path, MapFormat& outMap)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        LOG(MAP_UTILS_LOG, Error, std::format("Failed to open file: [{}]", path));
        return false;
    }

    file.read(reinterpret_cast<char*>(&outMap.width), sizeof(int32_t));
    file.read(reinterpret_cast<char*>(&outMap.height), sizeof(int32_t));

    int32_t sectorsCount = 0;
    file.read(reinterpret_cast<char*>(&sectorsCount), sizeof(int32_t));
    outMap.sectors.resize(sectorsCount);

    for (SectorData& sector : outMap.sectors)
    {
        file.read(reinterpret_cast<char*>(&sector.hasCost), sizeof(bool));
        if (sector.hasCost)
        {
            sector.costBuffer = new uint8_t[SECTOR_SIZE];
            file.read(reinterpret_cast<char*>(sector.costBuffer), SECTOR_SIZE);
        }
    }

    file.close();

    return true;
}

void MapUtils::ExportMap(const std::string& path, const MapFormat& map)
{
    std::ofstream stream(path, std::ios::binary);

    if (stream.is_open())
    {
        stream.write(reinterpret_cast<const char*>(&map.width), sizeof(int32_t));
        stream.write(reinterpret_cast<const char*>(&map.height), sizeof(int32_t));

        const int32_t sectorsCount = map.sectors.size();
        stream.write(reinterpret_cast<const char*>(&sectorsCount), sizeof(int32_t));

        for (const SectorData& sector : map.sectors)
        {
            stream.write(reinterpret_cast<const char*>(&sector.hasCost), sizeof(bool));
            if (sector.hasCost)
            {
                stream.write(reinterpret_cast<const char*>(sector.costBuffer), SECTOR_SIZE);
            }
        }

        stream.close();
    }
}
