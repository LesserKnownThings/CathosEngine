#include "RenderPipeline.h"
#include "Rendering/VkContext.h"

#include <iostream>
#include <vulkan/vulkan_core.h>

RenderPipeline::~RenderPipeline()
{
}

RenderPipeline::RenderPipeline(const VkContext& inContext) : context(inContext)
{
}

void RenderPipeline::Destroy()
{
    for (DescriptorSetLayoutInfo& layoutInfo : descriptorLayouts)
    {
        vkDestroyDescriptorSetLayout(context.device, layoutInfo.layout, nullptr);
    }

    vkDestroyPipeline(context.device, pipeline, nullptr);
    vkDestroyPipelineLayout(context.device, pipelineLayout, nullptr);
}

void RenderPipeline::Bind(VkCommandBuffer buffer)
{
    vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

VkShaderModule RenderPipeline::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    createInfo.pNext = nullptr;

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(context.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        std::cerr << "Failed to create shader module!" << std::endl;
    }

    return shaderModule;
}
