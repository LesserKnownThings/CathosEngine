#include "Frame.h"
#include "VkContext.h"

#include <iostream>

Frame::Frame(const VkContext &context)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = context.graphicsCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        std::cerr << "Failed to allocate command buffers!" << std::endl;
    }

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) !=
            VK_SUCCESS ||
        vkCreateSemaphore(context.device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) !=
            VK_SUCCESS ||
        vkCreateFence(context.device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
    {
        std::cerr << "Failed to create semaphores!" << std::endl;
    }
}

void Frame::Destroy(const VkContext &context)
{
    vkDestroySemaphore(context.device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(context.device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(context.device, inFlightFence, nullptr);
}
