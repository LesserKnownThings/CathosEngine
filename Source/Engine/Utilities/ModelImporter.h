#pragma once

#include <string>
#include <vector>

struct MeshData;

class ModelImporter
{
  public:
    static std::vector<MeshData> ImportModel(const std::string& path);
};