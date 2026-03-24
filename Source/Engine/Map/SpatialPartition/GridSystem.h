#pragma once

#include <cstdint>

struct MapFormat;

class GridSystem
{
  public:
    void CreateGrid(const MapFormat& mapFormat);
    void FreeGrid();

    int32_t GetGridWidth() const { return width; }

    static MapFormat CreateDebugMapFormat();

  private:
    int32_t width;
};