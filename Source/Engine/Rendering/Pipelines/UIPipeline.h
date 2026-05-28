#pragma once

#include "Rendering/Pipelines/RenderPipeline.h"
#include <string>

class UIPipeline : public RenderPipeline
{
  public:
    UIPipeline(const VkContext& inContext) : RenderPipeline(inContext) {}
    void Initialize() override;

  private:
    const std::string shaderPath = "Assets/Engine/Shaders/UI";
};