#pragma once

#include "Rendering/VkContext.h"

#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float4.hpp>
#include <vector>
#include <volk.h>
#include <vulkan/vulkan_core.h>

enum class EPipelineType : uint8_t
{
    PBR,
};

struct DescriptorBindingInfo
{
    uint32_t binding = 0;
    size_t type;
    size_t stage;
};

struct DescriptorSetLayoutInfo
{
    VkDescriptorSetLayout layout;
    uint32_t setIndex = 0;
};

struct SharedConstants
{
    alignas(16) glm::mat4 model;
};

struct CameraUV
{
    alignas(16) glm::vec4 uv;
};

class RenderPipeline
{
  public:
    virtual ~RenderPipeline();
    RenderPipeline(const VkContext& inContext);
    virtual void Destroy();

    virtual EPipelineType GetType() const = 0;

    virtual void Bind(VkCommandBuffer buffer);
    VkPipelineLayout GetLayout() const
    {
        return pipelineLayout;
    }

  protected:
    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    const VkContext& context;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    std::vector<DescriptorBindingInfo> descriptorBindings;
    std::vector<DescriptorSetLayoutInfo> descriptorLayouts;
};