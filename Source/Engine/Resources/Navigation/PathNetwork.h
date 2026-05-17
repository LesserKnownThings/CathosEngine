#pragma once

#include <cstdint>
#include <map>
#include <vector>

struct PathNetwork
{
    std::vector<int32_t> path;
    float lastUsedTime;
};

struct PathNetworkMap
{
    std::map<int32_t /*sector index*/, PathNetwork> networks;
};