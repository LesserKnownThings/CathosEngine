#pragma once

#include "RenderPipeline.h"
#include "Rendering/VkContext.h"

#include <array>
#include <string>
#include <vulkan/vulkan_core.h>

class PBRPipeline : public RenderPipeline
{
  public:
    PBRPipeline(const VkContext& inContext);

    virtual void Bind(VkCommandBuffer buffer) override;

    virtual EPipelineType GetType() const override;

  private:
    const std::string shaderPath = "Data/Shaders/PBR";

    std::array<VkDescriptorSet, 1> descriptorSets;

    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT,
                                                  VK_DYNAMIC_STATE_SCISSOR };
};