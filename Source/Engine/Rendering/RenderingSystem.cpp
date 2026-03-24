#include "RenderingSystem.h"
#include "BindingDefinitions.h"
#include "Callme/CallMe.Event.h"
#include "Components/Transform.h"
#include "Debug/DebugSystem.h"
#include "Frame.h"
#include "Game/Camera.h"
#include "InputManager.h"
#include "Pipelines/PBRPipeline.h"
#include "Rendering/Pipelines/RenderPipeline.h"
#include "Rendering/VkData.h"
#include "Resources/MaterialMesh3D.h"
#include "Resources/Model.h"
#include "Resources/Texture.h"
#include "TaskScheduler.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <entt/core/fwd.hpp>
#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entt.hpp>
#include <entt/resource/resource.hpp>
#include <glm/common.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <limits>
#include <map>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1004000
#include <vk_mem_alloc.h>

#define VOLK_IMPLEMENTATION
#include <volk.h>

#if MAP_EDITOR
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#endif

constexpr std::string VULKAN_LOG = "Vulkan";

uint32_t RenderingSystem::MIN_UNIFORM_ALIGNMENT = 64;

namespace Utilities
{
VKAPI_ATTR VkBool32 VKAPI_CALL
DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}
} // namespace Utilities

RenderingSystem& RenderingSystem::Get()
{
    static RenderingSystem instance{};
    return instance;
}

DescriptorRegistry RenderingSystem::descriptorRegistry = {};

bool RenderingSystem::Initialize(int32_t inWidth, int32_t inHeight)
{
    if (inWidth != 0 && inHeight != 0)
    {
        width = inWidth;
        height = inHeight;
    }

    const char* driver = SDL_GetCurrentVideoDriver();

    window = SDL_CreateWindow(applicationName.c_str(), width, height,
                              SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    if (window == nullptr)
    {
        const char* message = SDL_GetError();
        std::cerr << message << std::endl;
        return false;
    }

    InputManager& im = InputManager::Get();
    windowResizedHandle = im.onWindowResize.subscribe(CallMe::fromMethod<&RenderingSystem::HandleWindowResized>(this));
    windowMinimizedHandle = im.onWindowMinimized.subscribe(CallMe::fromMethod<&RenderingSystem::HandleWindowMinimized>(this));

    return InitializeVulkan();
}

void RenderingSystem::Shutdown()
{
    CleanupSwapChain();

    for (const auto& kvp : descriptorRegistry.descriptors)
    {
        DestroyDescriptorSetLayout(kvp.second.layout);
    }

    for (const auto& kvp : descriptorRegistry.mappedBuffers)
    {
        DestroyBuffer(kvp.second);
    }

    for (Frame& renderFrame : renderFrames)
    {
        renderFrame.Destroy(context);
    }
    renderFrames.clear();

    for (auto& pipeline : pipelines)
    {
        pipeline.second->Destroy();
        delete pipeline.second;
    }
    pipelines.clear();

    CleanupPendingDestroyBuffers();

    vkDestroyDescriptorPool(context.device, context.descriptorPool, nullptr);
    vkDestroyDescriptorPool(context.device, context.bindlessPool, nullptr);
    vkDestroyCommandPool(context.device, context.graphicsCommandPool, nullptr);
    vkDestroyCommandPool(context.device, context.transferCommandPool, nullptr);
    vkDestroyRenderPass(context.device, context.renderPass, nullptr);
    vkDestroyRenderPass(context.device, context.additivePass, nullptr);
    vkDestroySurfaceKHR(context.instance, context.surface, nullptr);
    vmaDestroyAllocator(context.allocator);
    vkDestroyDevice(context.device, nullptr);
    vkDestroyInstance(context.instance, nullptr);
    SDL_DestroyWindow(window);
}

void RenderingSystem::DestroyBuffer(AllocatedBuffer buffer)
{
    buffersPendingDelete.push_back(buffer);
}

void RenderingSystem::DestroyTexture(AllocatedTexture texture)
{
    // TODO like for the creation, this needs to live in a custom type of texture
    // if (texture.stagingBuffer != nullptr)
    // {
    //     DestroyBuffer(*texture.stagingBuffer);
    // }
    imagesPendingDelete.push_back(texture);
}

void RenderingSystem::DestroyDescriptorSetLayout(const VkDescriptorSetLayout layout)
{
    vkDestroyDescriptorSetLayout(context.device, layout, nullptr);
}

const Frame& RenderingSystem::GetCurrentFrame() const
{
    return renderFrames[currentFrame];
}

void RenderingSystem::BeginFrame()
{
    const Frame& frame = GetCurrentFrame();

    vkWaitForFences(context.device, 1, &frame.inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(context.device, 1, &frame.inFlightFence);

    vkAcquireNextImageKHR(context.device, context.swapChain, UINT64_MAX,
                          frame.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(frame.commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = context.renderPass;
    renderPassInfo.framebuffer = swapChainData.swapChainFramebuffers[imageIndex];

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = context.swapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { { 0.1f, 0.1f, 0.1f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(frame.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(context.swapChainExtent.height);
    viewport.width = static_cast<float>(context.swapChainExtent.width);
    viewport.height = -static_cast<float>(context.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(frame.commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = context.swapChainExtent;
    vkCmdSetScissor(frame.commandBuffer, 0, 1, &scissor);
}

void RenderingSystem::Draw(entt::registry& registry, float alpha)
{
    const Frame& currentFrame = GetCurrentFrame();
    VkCommandBuffer buffer = currentFrame.commandBuffer;

    auto camView = registry.view<CameraGlobalTransform>();
    auto updateCamFunc = [&](const CameraGlobalTransform& global)
    {
        UpdateCameraMatrix(global.projectionView);
    };
    camView.each(updateCamFunc);

    auto it = pipelines.find(EPipelineType::PBR);
    if (it != pipelines.end())
    {
        RenderPipeline* pipeline = it->second;
        pipeline->Bind(buffer);

        struct RenderCommand
        {
            MaterialMesh3DHandle* material;
            Model* mesh;
            entt::entity entity;

            bool operator<(const RenderCommand& other) const
            {
                if (material != other.material)
                    return material < other.material;
                return mesh < other.mesh;
            }
        };

        auto view = registry.view<Mesh, MaterialMesh3D, RenderTransform>();
        const auto& meshStorage = registry.storage<Mesh>();

        const entt::entity* entities = meshStorage.data();
        const int32_t storageSize = meshStorage.size();

        int32_t draws = 1;
        if (storageSize > MAX_INSTANCE_BUFFER)
        {
            draws = storageSize / MAX_INSTANCE_BUFFER;
        }

        static std::vector<RenderCommand> queue(MAX_INSTANCE_BUFFER);

        auto createSortTable = [&view, &entities](int32_t start, int32_t end)
        {
            for (int32_t i = start; i < end; ++i)
            {
                const Mesh& mesh = view.get<Mesh>(entities[i]);
                const MaterialMesh3D& material = view.get<MaterialMesh3D>(entities[i]);

                queue[i] = std::move(RenderCommand{
                    material.handle.handle().get(),
                    mesh.handle.handle().get(),
                    entities[i] });
            }
        };

        TaskScheduler& scheduler = TaskScheduler::Get();
        scheduler.ParallelForSync(storageSize, createSortTable);

        std::sort(queue.begin(), queue.begin() + storageSize);

        static std::vector<glm::mat4> models;
        static std::vector<MaterialData> materialData;
        models.resize(storageSize);
        materialData.resize(storageSize);

        // TODO Frustum culling

        auto processFunc = [&view, &alpha](int32_t start, int32_t end)
        {
            for (int32_t i = start; i < end; ++i)
            {
                RenderTransform& render = view.get<RenderTransform>(queue[i].entity);
                MaterialMesh3D& mat = view.get<MaterialMesh3D>(queue[i].entity);

                const glm::quat prevRot = render.prevRot;
                const glm::quat currRot = render.currentRot;

                glm::vec3 interpolatedPos = glm::mix(render.prevPos, render.currentPos, alpha);
                glm::vec3 interpolatedScale = glm::mix(render.prevScale, render.currentScale, alpha);
                glm::quat interpolatedRot = glm::slerp(prevRot, currRot, alpha);
                interpolatedRot = glm::normalize(interpolatedRot);

                glm::mat4 model = glm::translate(glm::mat4(1.0f), interpolatedPos);
                model = model * glm::mat4_cast(interpolatedRot);
                model = glm::scale(model, interpolatedScale);
                models[i] = model;

                materialData[i] = MaterialData{
                    mat.color,
                    glm::ivec4(mat.handle->textureHandle->textureIndex, glm::ivec3(0))
                };
            }
        };

        scheduler.ParallelForSync(storageSize, processFunc);

        auto it = descriptorRegistry.mappedBuffers.find({ InstanceBinding::SET, InstanceBinding::MODELS });
        if (it != descriptorRegistry.mappedBuffers.end())
        {
            uint8_t* instanceBuffer = static_cast<uint8_t*>(it->second.allocationInfo.pMappedData);
            memcpy(instanceBuffer, models.data(), models.size() * sizeof(glm::mat4));
        }

        // TODO maybe combine these 2?
        // But if I combine them I'll have to create the descriptor in both shader stages...
        // And also the data will become weird...
        auto matBufferIt = descriptorRegistry.mappedBuffers.find({ InstanceBinding::SET, InstanceBinding::MATERIALS });
        if (matBufferIt != descriptorRegistry.mappedBuffers.end())
        {
            uint8_t* instanceBuffer = static_cast<uint8_t*>(matBufferIt->second.allocationInfo.pMappedData);
            memcpy(instanceBuffer, materialData.data(), materialData.size() * sizeof(MaterialData));
        }

        int32_t batchStart = 0;

        entt::resource<Model> currentMeshHandle;
        entt::resource<MaterialMesh3DHandle> currentMaterialHandle;
        int32_t indicesCount = 0;

        while (batchStart < storageSize)
        {
            const Mesh& mesh = view.get<Mesh>(queue[batchStart].entity);
            const MaterialMesh3D& material = view.get<MaterialMesh3D>(queue[batchStart].entity);

            if (mesh.handle != currentMeshHandle)
            {
                currentMeshHandle = mesh.handle;
                MeshGPUData& gpuData = mesh.handle->meshes[mesh.meshIndex];

                VkDeviceSize zeroOffset = 0;

                vkCmdBindVertexBuffers(buffer, 0, 1, &gpuData.vertices.buffer, &zeroOffset);
                vkCmdBindIndexBuffer(buffer, gpuData.indices.buffer, 0, VK_INDEX_TYPE_UINT32);

                indicesCount = gpuData.indicesCount;
            }

            // if (material.handle != currentMaterialHandle)
            // {
            //     currentMaterialHandle = material.handle;
            //     AllocatedTexture textures[] = { material.handle->textureHandle->data.renderTexture };
            //     pipeline->UpdateMaterial(textures, 1);
            // }

            int32_t batchSize = 1;
            while (batchStart + batchSize < storageSize)
            {
                const Mesh& otherMesh = view.get<Mesh>(queue[batchStart + batchSize].entity);
                const MaterialMesh3D& otherMaterial = view.get<MaterialMesh3D>(queue[batchStart + batchSize].entity);

                if (otherMesh.handle != mesh.handle || otherMaterial.handle != material.handle)
                    break;

                batchSize++;
            }

            vkCmdDrawIndexed(buffer, indicesCount, batchSize, 0, 0, batchStart);

            batchStart += batchSize;
        }
    }
}

void RenderingSystem::EndFrame()
{
    const Frame& frame = GetCurrentFrame();

    vkCmdEndRenderPass(frame.commandBuffer);

    if (vkEndCommandBuffer(frame.commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &frame.imageAvailableSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &frame.renderFinishedSemaphore;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.commandBuffer;

    if (vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, frame.inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &frame.renderFinishedSemaphore;

    std::array<VkSwapchainKHR, 1> swapChains = { context.swapChain };
    presentInfo.swapchainCount = swapChains.size();
    presentInfo.pSwapchains = swapChains.data();
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(context.presentQueue, &presentInfo);

    VkResult fenceStatus = vkGetFenceStatus(context.device, GetCurrentFrame().inFlightFence);
    if (fenceStatus == VK_SUCCESS)
    {
        CleanupPendingDestroyBuffers();
    }
}

bool RenderingSystem::InitializeVulkan()
{
    bool success = CreateVulkanInstance();
    success &= CreateSurface();
    success &= PickPhysicalDevice();
    success &= CreateLogicalDevice();
    success &= CreateMemoryAllocator();
    success &= CreateSwapChain();
    success &= CreateRenderPass();
    success &= CreateFrameBuffers();
    success &= CreateCommandPools();
    success &= CreateDescriptorPool();

    if (!success)
        return false;

    CreateRenderFrames();
    CreateDescriptorRegistry();
    CreateRenderPipelines();
    SetupDebugMessenger();

    return true;
}

bool RenderingSystem::CheckValidationSupport() const
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

bool RenderingSystem::CreateVulkanInstance()
{
    if (volkInitialize() != VK_SUCCESS)
    {
        std::cerr << "Failed to initialize volk!" << std::endl;
        return false;
    }

    // TODO move this to a utility class for debugging
    if (enableValidationLayers && !CheckValidationSupport())
    {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t extensionCount = 0;
    const char* const* extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

    for (int32_t i = 0; i < extensionCount; ++i)
    {
        instanceExtensions.emplace_back(extensions[i]);
    }

    createInfo.enabledExtensionCount = instanceExtensions.size();
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    const VkResult result = vkCreateInstance(&createInfo, nullptr, &context.instance);
    if (result != VK_SUCCESS)
    {
        std::cerr << "Failed to create vulkan instance!" << std::endl;
        return false;
    }

    volkLoadInstance(context.instance);
    return true;
}

bool RenderingSystem::CreateSurface()
{
    if (!SDL_Vulkan_CreateSurface(window, context.instance, nullptr, &context.surface))
    {
        std::cerr << "Failed to create window surface!" << std::endl;
        return false;
    }
    return true;
}

bool RenderingSystem::PickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(context.instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        std::cerr << "Failed to find GPUs with Vulkan support!" << std::endl;
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(context.instance, &deviceCount, devices.data());

    std::multimap<int32_t, VkPhysicalDevice> deviceRatings;

    for (VkPhysicalDevice device : devices)
    {
        if (IsDeviceSuitable(device))
        {
            int32_t score = RateDeviceSuitability(device);
            deviceRatings.emplace(std::make_pair(score, device));
        }
    }

    context.physicalDevice = deviceRatings.rbegin()->second;
    context.msaaSamples = GetMaxUsableSampleCount();

    if (context.physicalDevice == VK_NULL_HANDLE)
    {
        std::cerr << "Failed to find a suitable GPU!" << std::endl;
        return false;
    }

    // Set device alignments
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(context.physicalDevice, &deviceProperties);

    MIN_UNIFORM_ALIGNMENT = deviceProperties.limits.minUniformBufferOffsetAlignment;

    return true;
}

bool RenderingSystem::CreateLogicalDevice()
{
    context.familyIndices = FindQueueFamilies(context.physicalDevice);
    QueueFamilyIndices& indices = context.familyIndices;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),
                                               indices.presentFamily.value() };
    if (indices.transferFamily.has_value())
    {
        uniqueQueueFamilies.emplace(indices.transferFamily.value());
    }

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = VK_TRUE;
    deviceFeatures.geometryShader = VK_TRUE;

    // Enable indexing for textures
    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    indexingFeatures.runtimeDescriptorArray = VK_TRUE;
    indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &indexingFeatures;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(context.physicalDevice, &createInfo, nullptr, &context.device) != VK_SUCCESS)
    {
        std::cerr << "Failed to create logical device!" << std::endl;
        return false;
    }

    vkGetDeviceQueue(context.device, indices.graphicsFamily.value(), 0, &context.graphicsQueue);
    vkGetDeviceQueue(context.device, indices.presentFamily.value(), 0, &context.presentQueue);

    if (indices.SupportsTransfer())
    {
        vkGetDeviceQueue(context.device, indices.transferFamily.value(), 0, &context.transferQueue);
    }

    volkLoadDevice(context.device);

    return true;
}

bool RenderingSystem::CreateMemoryAllocator()
{
    VmaVulkanFunctions vmaFunctions = {};
    vmaFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vmaFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorCreateInfo.physicalDevice = context.physicalDevice;
    allocatorCreateInfo.device = context.device;
    allocatorCreateInfo.instance = context.instance;
    allocatorCreateInfo.pVulkanFunctions = &vmaFunctions;

    if (vmaCreateAllocator(&allocatorCreateInfo, &context.allocator) != VK_SUCCESS)
    {
        std::cerr << "Failed to create memory allocator!";
        return false;
    }

    return true;
}

bool RenderingSystem::CreateSwapChain()
{
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(context.physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    // Max supported swapchain is double buffer
    imageCount = std::clamp<uint32_t>(imageCount, 0, MAX_SWAPCHAIN_IMAGES);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = context.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(context.physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(context.device, &createInfo, nullptr, &context.swapChain) !=
        VK_SUCCESS)
    {
        std::cerr << "failed to create swap chain!" << std::endl;
        return false;
    }

    vkGetSwapchainImagesKHR(context.device, context.swapChain, &imageCount, nullptr);
    swapChainData.Resize(imageCount);

    vkGetSwapchainImagesKHR(context.device, context.swapChain, &imageCount,
                            swapChainData.images.data());

    swapChainImageFormat = surfaceFormat.format;
    context.swapChainExtent = extent;

    for (size_t i = 0; i < swapChainData.views.size(); i++)
    {
        swapChainData.views[i] = CreateImageView(swapChainData.images[i], swapChainImageFormat,
                                                 VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }

    CreateColorResources();
    CreateDepthResources();

    return true;
}

bool RenderingSystem::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = context.msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = context.msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment,
                                                           colorAttachmentResolve };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(context.device, &renderPassInfo, nullptr, &context.renderPass) !=
        VK_SUCCESS)
    {
        std::cerr << "Failed to create render pass!" << std::endl;
        return false;
    }

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::array<VkAttachmentDescription, 3> additiveAttachments = { colorAttachment, depthAttachment,
                                                                   colorAttachmentResolve };

    VkSubpassDescription additiveSubpass{};
    additiveSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    additiveSubpass.colorAttachmentCount = 1;
    additiveSubpass.pColorAttachments = &colorAttachmentRef;
    additiveSubpass.pDepthStencilAttachment = &depthAttachmentRef;
    additiveSubpass.pResolveAttachments = &colorAttachmentResolveRef;

    VkRenderPassCreateInfo additivePassInfo{};
    additivePassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    additivePassInfo.attachmentCount = static_cast<uint32_t>(additiveAttachments.size());
    additivePassInfo.pAttachments = additiveAttachments.data();
    additivePassInfo.subpassCount = 1;
    additivePassInfo.pSubpasses = &additiveSubpass;

    if (vkCreateRenderPass(context.device, &additivePassInfo, nullptr, &context.additivePass) !=
        VK_SUCCESS)
    {
        std::cerr << "Failed to create additive render pass!" << std::endl;
        return false;
    }

    return true;
}

bool RenderingSystem::CreateFrameBuffers()
{
    for (size_t i = 0; i < swapChainData.swapChainFramebuffers.size(); i++)
    {
        std::array<VkImageView, 3> attachments = {
            swapChainData.colors[i].view, swapChainData.depths[i].view, swapChainData.views[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = context.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = context.swapChainExtent.width;
        framebufferInfo.height = context.swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(context.device, &framebufferInfo, nullptr,
                                &swapChainData.swapChainFramebuffers[i]) != VK_SUCCESS)
        {
            std::cerr << "Failed to create framebuffer!" << std::endl;
            return false;
        }
    }

    for (int32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        std::array<VkImageView, 3> attachments = {
            swapChainData.colors[i].view, swapChainData.depths[i].view, swapChainData.views[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = context.additivePass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = context.swapChainExtent.width;
        framebufferInfo.height = context.swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(context.device, &framebufferInfo, nullptr,
                                &additiveFrameBuffers[i]) != VK_SUCCESS)
        {
            std::cerr << "Failed to create additive framebuffer!" << std::endl;
            return false;
        }
    }

    return true;
}

bool RenderingSystem::CreateCommandPools()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(context.physicalDevice);

    VkCommandPoolCreateInfo graphicsPoolInfo{};
    graphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    graphicsPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    graphicsPoolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(context.device, &graphicsPoolInfo, nullptr,
                            &context.graphicsCommandPool) != VK_SUCCESS)
    {
        std::cerr << "Failed to create graphics command pool!" << std::endl;
        return false;
    }

    VkCommandPoolCreateInfo transferPoolinfo{};
    transferPoolinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transferPoolinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    transferPoolinfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

    if (vkCreateCommandPool(context.device, &transferPoolinfo, nullptr,
                            &context.transferCommandPool) != VK_SUCCESS)
    {
        std::cerr << "Failed to create transfer command pool!" << std::endl;
        return false;
    }

    return true;
}

bool RenderingSystem::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 4> poolSizes = {
        { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 500 },
          { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 500 },
          { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
          { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 } }
    };

    VkDescriptorPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.poolSizeCount = poolSizes.size();
    info.pPoolSizes = poolSizes.data();
    info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    info.maxSets = 10;

    if (vkCreateDescriptorPool(context.device, &info, nullptr, &context.descriptorPool) !=
        VK_SUCCESS)
    {
        std::cerr << "Failed to create descriptor pool!" << std::endl;
        return false;
    }

    // Creating texture pool separately because it needs different flags
    VkDescriptorPoolSize texturePoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURE_COUNT };

    VkDescriptorPoolCreateInfo texturePool{};
    texturePool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    texturePool.poolSizeCount = 1;
    texturePool.pPoolSizes = &texturePoolSize;
    texturePool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    texturePool.maxSets = 5;

    if (vkCreateDescriptorPool(context.device, &texturePool, nullptr, &context.bindlessPool) !=
        VK_SUCCESS)
    {
        std::cerr << "Failed to create bindless pool!" << std::endl;
        return false;
    }

    return true;
}

bool RenderingSystem::IsDeviceSuitable(VkPhysicalDevice device) const
{
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = FindQueueFamilies(device);

    bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate =
            !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.IsValid() && extensionsSupported && swapChainAdequate &&
           deviceFeatures.samplerAnisotropy;
}

bool RenderingSystem::CheckDeviceExtensionSupport(VkPhysicalDevice device) const
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void RenderingSystem::CreateRenderFrames()
{
    for (int32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        renderFrames.emplace_back(Frame{ context });
    }
}

void RenderingSystem::CreateRenderPipelines()
{
    PBRPipeline* basePipeline = new PBRPipeline(context);
    pipelines.emplace(basePipeline->GetType(), basePipeline);
}

void RenderingSystem::SetupDebugMessenger()
{
    if (!enableValidationLayers)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = Utilities::DebugCallback;
    createInfo.pUserData = nullptr;
}

void RenderingSystem::CleanupSwapChain()
{
    vkDeviceWaitIdle(context.device);

    for (int32_t i = 0; i < swapChainData.colors.size(); ++i)
    {
        vmaDestroyImage(context.allocator, swapChainData.colors[i].image,
                        swapChainData.colors[i].memory);

        vkDestroyImageView(context.device, swapChainData.colors[i].view, nullptr);

        vmaDestroyImage(context.allocator, swapChainData.depths[i].image,
                        swapChainData.depths[i].memory);

        vkDestroyImageView(context.device, swapChainData.depths[i].view, nullptr);
    }

    for (size_t i = 0; i < swapChainData.swapChainFramebuffers.size(); i++)
    {
        vkDestroyFramebuffer(context.device, swapChainData.swapChainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < swapChainData.views.size(); i++)
    {
        vkDestroyImageView(context.device, swapChainData.views[i], nullptr);
    }

    for (size_t i = 0; i < additiveFrameBuffers.size(); ++i)
    {
        vkDestroyFramebuffer(context.device, additiveFrameBuffers[i], nullptr);
    }

    vkDestroySwapchainKHR(context.device, context.swapChain, nullptr);
}

void RenderingSystem::CleanupPendingDestroyBuffers()
{
    for (AllocatedBuffer& bufferData : buffersPendingDelete)
    {
        vmaDestroyBuffer(context.allocator, bufferData.buffer, bufferData.memory);
    }
    buffersPendingDelete.clear();

    for (AllocatedTexture& texture : imagesPendingDelete)
    {
        vmaDestroyImage(context.allocator, texture.image, texture.memory);
        vkDestroyImageView(context.device, texture.view, nullptr);

        // In some contexts the sampler might be null
        if (texture.sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(context.device, texture.sampler, nullptr);
        }
    }
    imagesPendingDelete.clear();
}

void RenderingSystem::RecreateSwapChain()
{
    CleanupSwapChain();
    CreateSwapChain();
    CreateFrameBuffers();
}

glm::ivec2 RenderingSystem::GetWindowSize()
{
    SDL_GetWindowSize(window, &width, &height);
    return glm::ivec2(width, height);
}

void RenderingSystem::HandleWindowResized()
{
    SDL_GetWindowSize(window, &width, &height);
    RecreateSwapChain();
}

void RenderingSystem::HandleWindowMinimized()
{
    uint64_t flags = SDL_GetWindowFlags(window);

    // TODO block the main thread from running since netcode will be broken if this happens
    // TODO do not allow minized? I'm not sure I'll have to test, but I feel like this is not a good idea
    while (flags & SDL_WINDOW_MINIMIZED)
    {
        // GameEngine->GetInputSystem()->RunEvents();
        // flags = SDL_GetWindowFlags(window);
    }

    RecreateSwapChain();
}

void RenderingSystem::CreateColorResources()
{
    VkFormat colorFormat = swapChainImageFormat;

    for (int32_t i = 0; i < swapChainData.colors.size(); ++i)
    {
        VkImage image;
        VkImageView view;
        VmaAllocation memory;

        CreateImage(context.swapChainExtent.width, context.swapChainExtent.height, 1, 1,
                    context.msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);

        view = CreateImageView(image, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

        swapChainData.colors[i].image = image;
        swapChainData.colors[i].view = view;
        swapChainData.colors[i].memory = memory;
    }
}

void RenderingSystem::CreateDepthResources()
{
    VkFormat depthFormat = FindDepthFormat();

    for (int32_t i = 0; i < swapChainData.depths.size(); ++i)
    {
        VkImage image;
        VkImageView view;
        VmaAllocation memory;

        CreateImage(context.swapChainExtent.width, context.swapChainExtent.height, 1, 1,
                    context.msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, image, memory);

        view = CreateImageView(image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

        swapChainData.depths[i].image = image;
        swapChainData.depths[i].view = view;
        swapChainData.depths[i].memory = memory;
    }
}

void RenderingSystem::CreateDescriptorRegistry()
{
    CreateUniversalDescriptors();
    CreateInstanceDescriptors();
    CreateTextureDescriptors();
}

VkImageView RenderingSystem::CreateImageView(VkImage image, VkFormat format,
                                             VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = format;
    imageViewInfo.subresourceRange.aspectMask = aspectFlags;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = mipLevels;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(context.device, &imageViewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        std::cerr << "Failed to create image view!" << std::endl;
    }

    return imageView;
}

int32_t RenderingSystem::RateDeviceSuitability(VkPhysicalDevice device) const
{
    int32_t rating = 0;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        rating += 1000;
    }

    rating += deviceProperties.limits.maxImageDimension2D;

    return rating;
}

QueueFamilyIndices RenderingSystem::FindQueueFamilies(VkPhysicalDevice device) const
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (!indices.graphicsFamily.has_value() && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        if (!indices.transferFamily.has_value() &&
            (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
        {
            indices.transferFamily = i;
        }

        if (indices.graphicsFamily.has_value() && indices.transferFamily.has_value())
        {
            break;
        }

        i++;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, indices.graphicsFamily.value(), context.surface,
                                         &presentSupport);

    if (presentSupport)
    {
        indices.presentFamily = indices.graphicsFamily.value();
    }

    return indices;
}

SwapChainSupportDetails RenderingSystem::QuerySwapChainSupport(VkPhysicalDevice device) const
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, context.surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, context.surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, context.surface, &formatCount,
                                             details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, context.surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, context.surface, &presentModeCount,
                                                  details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR
RenderingSystem::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR
RenderingSystem::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D RenderingSystem::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    int32_t width, height;
    SDL_GetWindowSize(window, &width, &height);

    VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                    capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                     capabilities.maxImageExtent.height);

    return actualExtent;
}

VkFormat RenderingSystem::FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                              VkImageTiling tiling,
                                              VkFormatFeatureFlags features) const
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(context.physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                 (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported format!");
}

VkFormat RenderingSystem::FindDepthFormat() const
{
    return FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkSampleCountFlagBits RenderingSystem::GetMaxUsableSampleCount() const
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(context.physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    if (counts & VK_SAMPLE_COUNT_64_BIT)
    {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT)
    {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT)
    {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT)
    {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT)
    {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT)
    {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

void RenderingSystem::CreateImage(uint32_t width, uint32_t height, uint32_t layers,
                                  uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                                  VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                                  VmaAllocationCreateFlags memoryFlags, VkImage& outImage,
                                  VmaAllocation& outMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = layers;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo memoryInfo{};
    memoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
    memoryInfo.flags = memoryFlags;

    if (vmaCreateImage(context.allocator, &imageInfo, &memoryInfo, &outImage, &outMemory,
                       nullptr) != VK_SUCCESS)
    {
        std::cerr << "Failed to create image!" << std::endl;
    }
}

void RenderingSystem::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags bufferUsage,
                                   VmaMemoryUsage usage, VmaAllocationCreateFlags flags,
                                   VkBuffer& buffer, VmaAllocation& bufferMemory, VmaAllocationInfo* allocationInfo) const
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = bufferUsage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = usage;
    allocInfo.flags = flags;

    if (vmaCreateBuffer(context.allocator, &bufferInfo, &allocInfo, &buffer, &bufferMemory,
                        allocationInfo) != VK_SUCCESS)
    {
        LOG(VULKAN_LOG, Error, "Failed to create buffer!");
    }
}

void RenderingSystem::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
                                 VkDeviceSize srcOffset, VkDeviceSize dstOffset)
{
    SingleTimeCommandBuffer commandBuffer =
        BeginSingleTimeCommands(context.transferCommandPool, context.transferQueue);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer.commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
}

MeshGPUData RenderingSystem::CreateMesh(const MeshData& meshData)
{
    MeshGPUData mesh{};
    const VkDeviceSize verticesBufferSize = sizeof(Vertex) * meshData.vertices.size();
    VkDeviceSize indicesBufferSize = sizeof(uint32_t) * meshData.indices.size();
    const VkDeviceSize stagingBufferSize = verticesBufferSize + indicesBufferSize;

    VkBuffer stagingBuffer;
    VmaAllocation staginBufferMemory;

    CreateBuffer(
        stagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST,

        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer, staginBufferMemory);

    void* data;
    vmaMapMemory(context.allocator, staginBufferMemory, &data);
    memcpy(data, meshData.vertices.data(), static_cast<size_t>(verticesBufferSize));
    memcpy(reinterpret_cast<char*>(data) + verticesBufferSize, meshData.indices.data(),
           indicesBufferSize);
    vmaUnmapMemory(context.allocator, staginBufferMemory);

    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;

    VmaAllocation vertexMemory;
    VmaAllocation indexMemory;

    CreateBuffer(
        verticesBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, vertexBuffer, vertexMemory);

    CreateBuffer(
        indicesBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VMA_MEMORY_USAGE_AUTO, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, indexBuffer, indexMemory);

    // Vertex buffer
    CopyBuffer(stagingBuffer, vertexBuffer, verticesBufferSize, 0, 0);

    // Indices buffer
    CopyBuffer(stagingBuffer, indexBuffer, indicesBufferSize, verticesBufferSize, 0);

    mesh.vertices.buffer = vertexBuffer;
    mesh.vertices.memory = vertexMemory;
    mesh.verticesCount = meshData.vertices.size();

    mesh.indices.buffer = indexBuffer;
    mesh.indices.memory = indexMemory;
    mesh.indicesCount = meshData.indices.size();

    vmaDestroyBuffer(context.allocator, stagingBuffer, staginBufferMemory);
    return mesh;
}

void RenderingSystem::DestroyMesh(const MeshGPUData& mesh)
{
    DestroyBuffer(mesh.vertices);
    DestroyBuffer(mesh.indices);
}

#if MAP_EDITOR
void RenderingSystem::InitImGui()
{

    // auto func = [](const char* function_name, void* vulkan_instance)
    // {
    //     return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance*>(vulkan_instance)), function_name);
    // };

    // ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_4, func, &context.instance);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    style.ScaleAllSizes(main_scale);

    ImGui_ImplSDL3_InitForVulkan(window);

    ImGui_ImplVulkan_InitInfo info{};
    info.Instance = context.instance;
    info.PhysicalDevice = context.physicalDevice;
    info.Device = context.device;
    info.QueueFamily = context.familyIndices.graphicsFamily.value();
    info.Queue = context.graphicsQueue;
    info.DescriptorPoolSize = 500;
    info.MinImageCount = 2;
    info.ImageCount = 2;
    info.Allocator = nullptr;
    info.PipelineInfoMain.RenderPass = context.renderPass;
    info.PipelineInfoMain.Subpass = 0;
    info.PipelineInfoMain.MSAASamples = GetMaxUsableSampleCount();
    info.ApiVersion = VK_API_VERSION_1_4;

    ImGui_ImplVulkan_Init(&info);
}
#endif

void RenderingSystem::AllocateDescriptorSet(VkDescriptorSetLayout layout,
                                            VkDescriptorSet& outSet, VkDescriptorPool pool, const void* next)
{
    VkDescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool = pool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &layout;
    allocateInfo.pNext = next;

    if (vkAllocateDescriptorSets(context.device, &allocateInfo, &outSet) != VK_SUCCESS)
    {
        LOG(VULKAN_LOG, Error, "Failed to allocate descriptor set!");
    }
}

void RenderingSystem::CreateUniversalDescriptors()
{
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = UniversalBinding::CAMERA;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = UniversalBinding::BINDING_COUNT;
    createInfo.pBindings = &binding;

    Descriptor descriptor{};
    if (vkCreateDescriptorSetLayout(context.device, &createInfo, nullptr,
                                    &descriptor.layout) != VK_SUCCESS)
    {
        throw std::invalid_argument("Failed to create descriptor!");
    }

    AllocatedBuffer buffer;
    CreateBuffer(sizeof(glm::mat4) * MAX_FRAMES_IN_FLIGHT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_MAPPED_BIT, buffer.buffer, buffer.memory, &buffer.allocationInfo);

    AllocateDescriptorSet(descriptor.layout, descriptor.set, context.descriptorPool);
    UpdateDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, descriptor.set, buffer);

    descriptorRegistry.descriptors.emplace(UniversalBinding::INDEX, std::move(descriptor));
    descriptorRegistry.mappedBuffers.emplace(std::make_tuple(UniversalBinding::SET, UniversalBinding::CAMERA), std::move(buffer));
}

void RenderingSystem::CreateInstanceDescriptors()
{
    VkDescriptorSetLayoutBinding bindings[InstanceBinding::BINDING_COUNT];

    bindings[0].binding = InstanceBinding::MODELS;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    bindings[1].binding = InstanceBinding::MATERIALS;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = InstanceBinding::BINDING_COUNT;
    createInfo.pBindings = bindings;

    Descriptor descriptor{};
    if (vkCreateDescriptorSetLayout(context.device, &createInfo, nullptr,
                                    &descriptor.layout) != VK_SUCCESS)
    {
        throw std::invalid_argument("Failed to create descriptor!");
    }

    AllocateDescriptorSet(descriptor.layout, descriptor.set, context.descriptorPool);
    descriptorRegistry.descriptors.emplace(InstanceBinding::INDEX, std::move(descriptor));

    AllocatedBuffer modelsBuffer;
    CreateBuffer(sizeof(glm::mat4) * MAX_FRAMES_IN_FLIGHT * MAX_INSTANCE_BUFFER, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_MAPPED_BIT, modelsBuffer.buffer, modelsBuffer.memory, &modelsBuffer.allocationInfo);
    UpdateDescriptorSet(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptor.set, modelsBuffer, InstanceBinding::MODELS);

    descriptorRegistry.mappedBuffers.emplace(std::make_tuple(InstanceBinding::SET, InstanceBinding::MODELS), std::move(modelsBuffer));

    AllocatedBuffer materialsBuffer;
    CreateBuffer(sizeof(MaterialData) * MAX_FRAMES_IN_FLIGHT * MAX_INSTANCE_BUFFER, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_MAPPED_BIT, materialsBuffer.buffer, materialsBuffer.memory, &materialsBuffer.allocationInfo);
    UpdateDescriptorSet(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, descriptor.set, materialsBuffer, InstanceBinding::MATERIALS);

    descriptorRegistry.mappedBuffers.emplace(std::make_tuple(InstanceBinding::SET, InstanceBinding::MATERIALS), std::move(materialsBuffer));
}

void RenderingSystem::CreateTextureDescriptors()
{
    VkDescriptorSetLayoutBinding textureBinding{};
    textureBinding.binding = 0;
    textureBinding.descriptorCount = MAX_TEXTURE_COUNT;
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    textureBinding.pImmutableSamplers = nullptr;

    VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                     VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
    bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlags.bindingCount = 1;
    bindingFlags.pBindingFlags = &flags;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &textureBinding;
    createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    createInfo.pNext = &bindingFlags;

    Descriptor descriptor;
    if (vkCreateDescriptorSetLayout(context.device, &createInfo, nullptr,
                                    &descriptor.layout) != VK_SUCCESS)
    {
        throw std::invalid_argument("Failed to create texture descriptor!");
    }

    const uint32_t count = MAX_TEXTURE_COUNT;
    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    variableCountInfo.descriptorSetCount = 1;
    variableCountInfo.pDescriptorCounts = &count;

    AllocateDescriptorSet(descriptor.layout, descriptor.set, context.bindlessPool, &variableCountInfo);
    descriptorRegistry.descriptors.emplace(TextureBinding::INDEX, std::move(descriptor));
}

void RenderingSystem::UpdateDescriptorSet(VkDescriptorType type, VkDescriptorSet set,
                                          AllocatedBuffer buffer, uint32_t binding)
{
    VkDescriptorBufferInfo writeInfo{};
    writeInfo.buffer = buffer.buffer;
    writeInfo.offset = 0;
    writeInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.descriptorType = type;
    write.descriptorCount = 1;
    write.pBufferInfo = &writeInfo;

    vkUpdateDescriptorSets(context.device, 1, &write, 0, nullptr);
}

void RenderingSystem::UnmapMemory(VmaAllocation allocation)
{
    vmaUnmapMemory(context.allocator, allocation);
}

void RenderingSystem::UpdateCameraMatrix(const glm::mat4& projectionView)
{
    auto it = descriptorRegistry.mappedBuffers.find({ UniversalBinding::INDEX, 0 });
    if (it != descriptorRegistry.mappedBuffers.end())
    {
        AllocatedBuffer& buffer = it->second;

        uint8_t* data = static_cast<uint8_t*>(buffer.allocationInfo.pMappedData);

        uint32_t frameOffset = sizeof(glm::mat4) * currentFrame;

        memcpy(data + frameOffset, glm::value_ptr(projectionView),
               sizeof(glm::mat4));
    }
}

uint32_t RenderingSystem::GetOrCreateTextureIndex()
{
    if (cachedTextureIndices.empty())
    {
        return currentTextureIndex++;
    }
    uint32_t index = cachedTextureIndices.front();
    cachedTextureIndices.pop();
    return index;
}

void RenderingSystem::ReturnTextureIndex(uint32_t index)
{
    cachedTextureIndices.push(index);
}

void RenderingSystem::CreateSRGBATexture(Texture2D* texture, void* pixels)
{
    VkBuffer stagingBuffer;
    VmaAllocation stagingMemory;
    const int32_t channels = texture->data.channels;

    // TODO usually textures have 1byte channels, but still need to make sure here
    const VkDeviceSize textureSize = static_cast<VkDeviceSize>(texture->data.width) *
                                     static_cast<VkDeviceSize>(texture->data.height) * channels;

    CreateBuffer(textureSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT, stagingBuffer,
                 stagingMemory);

    void* data = nullptr;

    if (pixels != nullptr)
    {
        vmaMapMemory(context.allocator, stagingMemory, &data);
        memcpy(data, pixels, static_cast<size_t>(textureSize));
        vmaUnmapMemory(context.allocator, stagingMemory);
    }

    VkImage imageBuffer;
    VmaAllocation imageMemory;

    CreateImage(texture->data.width, texture->data.height, 1, 1, VK_SAMPLE_COUNT_1_BIT,
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, imageBuffer, imageMemory);

    SingleTimeCommandBuffer commandBuffer =
        BeginSingleTimeCommands(context.transferCommandPool, context.transferQueue);

    TransitionImageLayout(commandBuffer.commandBuffer, imageBuffer, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 0, 1);

    CopyBufferToImage(commandBuffer.commandBuffer, stagingBuffer, imageBuffer, 0, 0,
                      texture->data.width, texture->data.height, 0, 1);

    EndSingleTimeCommands(commandBuffer);

    TransitionImageOwnership(imageBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             context.familyIndices.transferFamily.value(),
                             context.familyIndices.graphicsFamily.value());

    commandBuffer = BeginSingleTimeCommands(context.graphicsCommandPool, context.graphicsQueue);
    TransitionImageLayout(commandBuffer.commandBuffer, imageBuffer,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 0, 1);
    EndSingleTimeCommands(commandBuffer);

    texture->renderTexture.image = imageBuffer;
    texture->renderTexture.memory = imageMemory;

    vmaDestroyBuffer(context.allocator, stagingBuffer, stagingMemory);

    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = imageBuffer;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(context.device, &imageViewInfo, nullptr, &texture->renderTexture.view) != VK_SUCCESS)
    {
        std::cerr << "Failed to create image view!" << std::endl;
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(context.physicalDevice, &deviceProperties);

    samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(context.device, &samplerInfo, nullptr, &texture->renderTexture.sampler) != VK_SUCCESS)
    {
        std::cerr << "Failed tocreate image sampler!" << std::endl;
    }

    auto texturesIt = descriptorRegistry.descriptors.find(TextureBinding::INDEX);

    if (texturesIt != descriptorRegistry.descriptors.end())
    {
        texture->textureIndex = GetOrCreateTextureIndex();
        VkWriteDescriptorSet write{};
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture->renderTexture.view;
        imageInfo.sampler = texture->renderTexture.sampler;

        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = texturesIt->second.set;
        write.dstBinding = 0;
        write.dstArrayElement = texture->textureIndex;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(context.device, 1, &write, 0, nullptr);
    }

    // TODO this needs to live in a custom texture instead of the base texture
    // if (keepStagingBuffer)
    // {
    //     outTexture.stagingBuffer = new AllocatedBuffer{};
    //     outTexture.stagingBuffer->buffer = stagingBuffer;
    //     outTexture.stagingBuffer->memory = stagingMemory;
    // }
}

SingleTimeCommandBuffer RenderingSystem::BeginSingleTimeCommands(VkCommandPool commandPool,
                                                                 VkQueue queue)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkFence fence;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (vkCreateFence(context.device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
    {
        std::cerr << "Failed to create fence for single time command buffer!" << std::endl;
    }

    SingleTimeCommandBuffer sCommandBuffer{};
    sCommandBuffer.commandBuffer = commandBuffer;
    sCommandBuffer.commandPool = commandPool;
    sCommandBuffer.queue = queue;
    sCommandBuffer.fence = fence;
    return sCommandBuffer;
}

void RenderingSystem::EndSingleTimeCommands(const SingleTimeCommandBuffer& commandBuffer)
{
    vkEndCommandBuffer(commandBuffer.commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer.commandBuffer;

    vkQueueSubmit(commandBuffer.queue, 1, &submitInfo, commandBuffer.fence);
    vkWaitForFences(context.device, 1, &commandBuffer.fence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(context.device, commandBuffer.fence, nullptr);
    vkFreeCommandBuffers(context.device, commandBuffer.commandPool, 1,
                         &commandBuffer.commandBuffer);
}

void RenderingSystem::TransitionImageOwnership(VkImage imageBuffer, VkImageLayout layout,
                                               uint32_t srcFamilyIndex, uint32_t dstFamilyIndex)
{
    SingleTimeCommandBuffer commandBuffer =
        BeginSingleTimeCommands(context.transferCommandPool, context.transferQueue);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = layout;
    barrier.newLayout = layout;
    barrier.srcQueueFamilyIndex = srcFamilyIndex;
    barrier.dstQueueFamilyIndex = dstFamilyIndex;
    barrier.image = imageBuffer;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = 0; // nothing to wait for on destination yet

    vkCmdPipelineBarrier(commandBuffer.commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);

    EndSingleTimeCommands(commandBuffer);

    SingleTimeCommandBuffer aquireCommandBuffer =
        BeginSingleTimeCommands(context.graphicsCommandPool, context.graphicsQueue);

    VkImageMemoryBarrier aquireBarrier{};
    aquireBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    aquireBarrier.oldLayout = layout;
    aquireBarrier.newLayout = layout;
    aquireBarrier.srcQueueFamilyIndex = srcFamilyIndex;
    aquireBarrier.dstQueueFamilyIndex = dstFamilyIndex;
    aquireBarrier.image = imageBuffer;
    aquireBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    aquireBarrier.subresourceRange.baseMipLevel = 0;
    aquireBarrier.subresourceRange.levelCount = 1;
    aquireBarrier.subresourceRange.baseArrayLayer = 0;
    aquireBarrier.subresourceRange.layerCount = 1;
    aquireBarrier.srcAccessMask = 0;
    aquireBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(aquireCommandBuffer.commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                         &aquireBarrier);

    EndSingleTimeCommands(aquireCommandBuffer);
}

void RenderingSystem::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,
                                            VkImageLayout oldLayout, VkImageLayout newLayout,
                                            uint32_t mipLevels, int32_t layerIndex,
                                            int32_t layerCount)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = layerIndex;
    barrier.subresourceRange.layerCount = layerCount;

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);
}

void RenderingSystem::CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer,
                                        VkImage image, int32_t offsetX, int32_t offsetY,
                                        uint32_t width, uint32_t height, uint32_t layerIndex,
                                        uint32_t layerCount)
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = layerIndex;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = { offsetX, offsetY, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &region);
}
