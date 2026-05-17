#include "PBRPipeline.h"
#include "Debug/DebugSystem.h"
#include "Rendering/BindingDefinitions.h"
#include "Rendering/DescriptorRegistry.h"
#include "Rendering/Pipelines/RenderPipeline.h"
#include "Rendering/RenderingSystem.h"
#include "Rendering/VkContext.h"
#include "Resources/Model.h"
#include "Utilities/FileHelper.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <glm/ext/vector_float4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

REGISTER_PIPELINE(PBRPipeline, PipelineType::PBR)

constexpr std::string PBR_LOG = "PBR";

constexpr int32_t PBR_DESCRIPTOR_SETS = 3;
constexpr int32_t PBR_SHADER_STAGES = 2;

void PBRPipeline::Initialize()
{
    // SHADER STAGES
    std::array<VkPipelineShaderStageCreateInfo, PBR_SHADER_STAGES> shaderStages{};

    // Vertex
    const std::string vertShaderPath = shaderPath + "/vert.spv";
    auto vertShaderCode = FileHelper::ReadFile(vertShaderPath, std::ios::binary | std::ios::ate);

    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.pNext = nullptr;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    shaderStages[0] = vertShaderStageInfo;

    // Fragment
    const std::string fragShaderPath = shaderPath + "/frag.spv";
    auto fragShaderCode = FileHelper::ReadFile(fragShaderPath, std::ios::binary | std::ios::ate);

    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.pNext = nullptr;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    shaderStages[1] = fragShaderStageInfo;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.pNext = nullptr;

    std::array<VkVertexInputBindingDescription, 1> bindingDescriptions{};
    std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions{};

    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, tangent);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, bitangent);

    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(Vertex, uv);

    attributeDescriptions[5].binding = 0;
    attributeDescriptions[5].location = 5;
    attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SINT;
    attributeDescriptions[5].offset = offsetof(Vertex, boneIDs);

    attributeDescriptions[6].binding = 0;
    attributeDescriptions[6].location = 6;
    attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[6].offset = offsetof(Vertex, weights);

    vertexInputInfo.vertexBindingDescriptionCount = bindingDescriptions.size();
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    // *************

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    inputAssembly.pNext = nullptr;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(context.swapChainExtent.width);
    viewport.height = static_cast<float>(context.swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = context.swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    viewportState.pNext = nullptr;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.rasterizationSamples = context.msaaSamples;
    multisampling.minSampleShading = 0.2f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
    multisampling.pNext = nullptr;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pNext = nullptr;

    std::array<VkDescriptorSetLayout, PBR_DESCRIPTOR_SETS> descriptorSetLayouts{};
    descriptorSets.resize(PBR_DESCRIPTOR_SETS);
    const DescriptorRegistry& registry = RenderingSystem::GetRegistry();

    Descriptor universal{};
    if (registry.GetDescriptor(UniversalBinding::INDEX, universal))
    {
        descriptorSetLayouts[0] = universal.layout;
        descriptorSets[0] = universal.set;
    }

    Descriptor instance{};
    if (registry.GetDescriptor(InstanceBinding::INDEX, instance))
    {
        descriptorSetLayouts[1] = instance.layout;
        descriptorSets[1] = instance.set;
    }

    Descriptor textures{};
    if (registry.GetDescriptor(TextureBinding::INDEX, textures))
    {
        descriptorSetLayouts[2] = textures.layout;
        descriptorSets[2] = textures.set;
    }

    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

    if (vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
        VK_SUCCESS)
    {
        LOG(PBR_LOG, Error, "Failed to create PBR pipeline layout!");
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};
    depthStencil.pNext = nullptr;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = shaderStages.size();
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = context.worldRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.pNext = nullptr;

    if (vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  &pipeline) != VK_SUCCESS)
    {
        LOG(PBR_LOG, Error, "Failed to create PBR pipeline!");
    }

    for (VkPipelineShaderStageCreateInfo& stage : shaderStages)
    {
        vkDestroyShaderModule(context.device, stage.module, nullptr);
    }
}
