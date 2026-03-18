#include "GridSystem.h"
#include <cstdint>
#include <cstdlib>
#include <vector>

const std::vector<uint8_t> GridSystem::CLEAR_COST(CHUNK_SIZE, 1);

void GridSystem::CreateGrid(int32_t size)
{
    gridSize = size * CHUNK_SIZE;

    const int32_t nSize = size * size;
}

void GridSystem::FreeGrid()
{
}