#pragma once

struct AssetPath;
struct MapFormat;

class MapImporter
{
  public:
    static bool ImportMap(const AssetPath& path, MapFormat& outMap);
};