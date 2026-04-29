#pragma once

#include <string>

struct MapFormat;

class MapUtils
{
  public:
    static bool ImportMap(const std::string& path, MapFormat& outMap);
    static void ExportMap(const std::string& path, const MapFormat& map);
};