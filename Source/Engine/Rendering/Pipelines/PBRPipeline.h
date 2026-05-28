#pragma once

#include "RenderPipeline.h"

#include <string>

struct VkContext;

class PBRPipeline : public RenderPipeline
{
  public:
    PBRPipeline(const VkContext& inContext) : RenderPipeline(inContext) {}
    void Initialize() override;

  private:
    const std::string shaderPath = "Assets/Engine/Shaders/PBR";
};