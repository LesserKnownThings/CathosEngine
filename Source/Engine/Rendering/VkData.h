#pragma once

#include <vk_mem_alloc.h>
#include <volk.h>

struct AllocatedBuffer
{
    VkBuffer buffer;
    VmaAllocation memory;
    VmaAllocationInfo allocationInfo;
};

struct AllocatedTexture
{
    VkImage image;
    VkImageView view;
    VkSampler sampler;
    VmaAllocation memory;
};