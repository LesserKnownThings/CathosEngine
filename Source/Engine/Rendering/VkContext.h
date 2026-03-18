#pragma once

#include <optional>
#include <vk_mem_alloc.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    std::optional<uint32_t> transferFamily;

    bool IsValid() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    bool SupportsTransfer() const
    {
        return transferFamily.has_value();
    }
};

struct VkContext
{
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkQueue transferQueue = VK_NULL_HANDLE;

    VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
    VkCommandPool transferCommandPool = VK_NULL_HANDLE;

    VkExtent2D swapChainExtent;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkRenderPass additivePass = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorPool bindlessPool;

    QueueFamilyIndices familyIndices;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocator allocator = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
};
