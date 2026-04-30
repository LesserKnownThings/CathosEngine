#pragma once

#include "Callme/CallMe.Event.h"
#include "Map/MapFormat.h"
#include <string>

class HeightmapEditor
{
  public:
    static HeightmapEditor& Get();
    void Generate(const std::string& heightmapTexturePath, const std::string& normalTexturePath);
    void LoadMap(const std::string& path);

    CallMe::Event<void(const MapFormat&)> onMapCreated;

    bool IsBaked() const { return isBaked; }
    const MapFormat& GetMap() const { return map; }

  private:
    void BakeMap();

    std::string heightmap;
    std::string normal;

    bool isBaked = false;

    MapFormat map{};
};