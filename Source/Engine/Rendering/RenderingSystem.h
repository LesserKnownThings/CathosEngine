#pragma once

#include "Callme/CallMe.Event.h"
#include "DescriptorRegistry.h"
#include "Frame.h"
#include "Pipelines/RenderPipeline.h"
#include "VkContext.h"
#include "VkData.h"

#include <SDL3/SDL_video.h>
#include <array>
#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_int2.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <vk_mem_alloc.h>
#include <volk.h>
#include <vulkan/vulkan_core.h>

struct View;
struct TextureData;
struct MeshData;
struct MeshGPUData;

constexpr int32_t MAX_FRAMES_IN_FLIGHT = 2;
// Only supporting double buffering for smoother movement
constexpr int32_t MAX_SWAPCHAIN_IMAGES = 3;

constexpr uint32_t NVIGIA_GPU = 0x10DE;
constexpr uint32_t AMD_GPU = 0x1002;
constexpr uint32_t INTEL_GPU = 0x8086;

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SingleTimeCommandBuffer
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
};

struct SwapChainData
{
    void Resize(uint32_t size)
    {
        swapChainFramebuffers.resize(size);
        images.resize(size);
        views.resize(size);
        depths.resize(size);
        colors.resize(size);
    }

    std::vector<VkFramebuffer> swapChainFramebuffers;

    std::vector<VkImage> images;
    std::vector<VkImageView> views;

    std::vector<AllocatedTexture> depths;
    std::vector<AllocatedTexture> colors;
};

struct RenderContext
{
    VkCommandBuffer commandBuffer;
    std::unordered_map<EPipelineType, RenderPipeline*> pipelines;
};

class RenderingSystem
{
  public:
    static RenderingSystem& Get();

    bool
    Initialize(int32_t inWidth = 0, int32_t inHeight = 0);
    void Shutdown();

    void BeginFrame();
    void Draw(entt::registry& registry, float alpha);
    void EndFrame();

    void DestroyBuffer(AllocatedBuffer buffer);
    void DestroyTexture(AllocatedTexture texture);
    void DestroyDescriptorSetLayout(const VkDescriptorSetLayout layout);

    template <typename T>
    void MapMemory(VmaAllocation allocation, T*& mappedBuffer)
    {
        vmaMapMemory(context.allocator, allocation, reinterpret_cast<void**>(&mappedBuffer));
    }
    void UnmapMemory(VmaAllocation allocation);

    void UpdateEntireChunkTexture(const AllocatedTexture& texture, void* data, int32_t width,
                                  int32_t height, int32_t layerIndex);
    void UpdateChunkTexture(const AllocatedTexture& texture, void* data, int32_t x, int32_t y,
                            uint32_t width, uint32_t height, int32_t layerIndex);

    void CreateTexture(const TextureData& textureData, void* pixels, AllocatedTexture& outTexture,
                       bool keepStagingBuffer = false);
    MeshGPUData CreateMesh(const MeshData& meshData);
    void DestroyMesh(const MeshGPUData& mesh);

    void UpdateCameraMatrix(const glm::mat4& projection, const glm::mat4& view);

    float GetScreenWidth() const
    {
        return width;
    }

    float GetScreenHeight() const
    {
        return height;
    }

    float GetAspectRatio() const
    {
        return static_cast<float>(width) / static_cast<float>(height);
    }

    glm::ivec2 GetWindowSize();

    static const DescriptorRegistry& GetRegistry()
    {
        return descriptorRegistry;
    }

  private:
    static uint32_t MIN_UNIFORM_ALIGNMENT;

    const Frame& GetCurrentFrame() const;

    bool InitializeVulkan();
    bool CheckValidationSupport() const;

    bool CreateVulkanInstance();
    bool CreateSurface();
    bool PickPhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateMemoryAllocator();
    bool CreateSwapChain();
    bool CreateRenderPass();
    bool CreateFrameBuffers();
    bool CreateCommandPools();
    bool CreateDescriptorPool();

    bool IsDeviceSuitable(VkPhysicalDevice device) const;
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;

    void CreateRenderFrames();
    void CreateRenderPipelines();
    void SetupDebugMessenger();

    void CleanupSwapChain();
    void CleanupPendingDestroyBuffers();
    void RecreateSwapChain();

    void CreateColorResources();
    void CreateDepthResources();
    void CreateDescriptorRegistry();

    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
                                uint32_t mipLevels);

    int32_t RateDeviceSuitability(VkPhysicalDevice device) const;
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
    VkSurfaceFormatKHR
    ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR
    ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features) const;
    VkFormat FindDepthFormat() const;
    VkSampleCountFlagBits GetMaxUsableSampleCount() const;

    SingleTimeCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool, VkQueue queue);
    void EndSingleTimeCommands(const SingleTimeCommandBuffer& commandBuffer);
    void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
                               VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels,
                               int32_t layerIndex, int32_t layerCount);
    void TransitionImageOwnership(VkImage imageBuffer, VkImageLayout layout,
                                  uint32_t srcFamilyIndex, uint32_t dstFamilyIndex);
    void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image,
                           int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height,
                           uint32_t layerIndex, uint32_t layerCount);

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage usage,
                      VmaAllocationCreateFlags flags, VkBuffer& buffer,
                      VmaAllocation& bufferMemory, VmaAllocationInfo* allocationInfo = nullptr) const;
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
                    VkDeviceSize srcOffset, VkDeviceSize dstOffset);

    void CreateImage(uint32_t width, uint32_t height, uint32_t layers, uint32_t mipLevels,
                     VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage, VmaAllocationCreateFlags memoryFlags,
                     VkImage& outImage, VmaAllocation& outMemory);

    void CreateUniversalDescriptors();
    void CreateInstanceDescriptors();

    void CreateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSet& outSet);
    void UpdateDescriptorSet(VkDescriptorType type, VkDescriptorSet set, AllocatedBuffer buffer,
                             uint32_t binding = 0);

    void HandleWindowResized();
    void HandleWindowMinimized();

    // TODO Move this to a viewport struct ?
    int32_t width = 900;
    int32_t height = 500;

    SDL_Window* window = nullptr;
    const std::string applicationName = "RTS";

    std::optional<CallMe::Subscription> windowResizedHandle;
    std::optional<CallMe::Subscription> windowMinimizedHandle;

    VkContext context = {};
    static DescriptorRegistry descriptorRegistry;
    VkFormat swapChainImageFormat;
    SwapChainData swapChainData;

    uint32_t currentFrame = 0;
    uint32_t imageIndex = 0;

    std::vector<Frame> renderFrames;
    std::array<VkFramebuffer, MAX_FRAMES_IN_FLIGHT> additiveFrameBuffers;

    std::unordered_map<EPipelineType, RenderPipeline*> pipelines;

    std::vector<AllocatedBuffer> buffersPendingDelete;
    std::vector<AllocatedTexture> imagesPendingDelete;

    std::vector<const char*> instanceExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };

    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    // **** Editor *****
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

    const bool enableValidationLayers = true;

    friend class World;
};