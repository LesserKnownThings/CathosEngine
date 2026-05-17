#pragma once

#include "Rendering/VkData.h"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

struct SwapChainData
{
    void Resize(uint32_t size)
    {
        availableImagesCount = size;
        geometryFrameBuffers.resize(size);
        uiFrameBuffers.resize(size);
        images.resize(size);
        views.resize(size);
        depths.resize(size);
        colors.resize(size);
    }

    void Clear(VkDevice device, VmaAllocator allocator);

    std::vector<VkFramebuffer> geometryFrameBuffers;
    std::vector<VkFramebuffer> uiFrameBuffers;

    std::vector<VkImage> images;
    std::vector<VkImageView> views;

    std::vector<AllocatedTexture> depths;
    std::vector<AllocatedTexture> colors;

    uint32_t availableImagesCount;
};