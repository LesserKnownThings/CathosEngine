#pragma once

#include <cstdint>
#include <map>
#include <vector>

struct PortalIndices
{
    std::vector<int32_t> horizontal;
    std::vector<int32_t> vertical;
};

struct PortalMap
{
    std::map<int32_t, PortalIndices> portals;
};