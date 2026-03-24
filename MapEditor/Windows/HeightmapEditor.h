#pragma once

#include "Callme/CallMe.Event.h"
#include "Map/MapFormat.h"
#include <string>

class HeightmapEditor
{
  public:
    static HeightmapEditor& Get();
    void Generate(const std::string& heightmapTexturePath, const std::string& normalTexturePath);

    CallMe::Event<void(const MapFormat&)> onMapCreated;

  private:
    void BakeMap();

    std::string heightmap;
    std::string normal;

    MapFormat map{};
};