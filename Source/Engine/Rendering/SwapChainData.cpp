#include "SwapChainData.h"

void SwapChainData::Clear(VkDevice device, VmaAllocator allocator)
{
    for (int32_t i = 0; i < colors.size(); ++i)
    {
        vmaDestroyImage(allocator, colors[i].image,
                        colors[i].memory);

        vkDestroyImageView(device, colors[i].view, nullptr);

        vmaDestroyImage(allocator, depths[i].image,
                        depths[i].memory);

        vkDestroyImageView(device, depths[i].view, nullptr);
    }

    for (int32_t i = 0; i < geometryFrameBuffers.size(); i++)
    {
        vkDestroyFramebuffer(device, geometryFrameBuffers[i], nullptr);
    }

    for (int32_t i = 0; i < uiFrameBuffers.size(); ++i)
    {
        vkDestroyFramebuffer(device, uiFrameBuffers[i], nullptr);
    }

    for (int32_t i = 0; i < views.size(); i++)
    {
        vkDestroyImageView(device, views[i], nullptr);
    }
}