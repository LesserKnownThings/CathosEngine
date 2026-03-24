#pragma once

#include <atomic>
#include <entt/entt.hpp>
#include <vector>

struct AtomicGrid
{
    int32_t width;
    int32_t height;

    std::vector<std::atomic<entt::entity>> heads;

    AtomicGrid(int w, int h, size_t count)
        : width(w),
          height(h),
          heads(count)
    {
    }

    AtomicGrid(const AtomicGrid&) = delete;
    AtomicGrid(AtomicGrid&&) = delete;
};

struct Grid
{
};
