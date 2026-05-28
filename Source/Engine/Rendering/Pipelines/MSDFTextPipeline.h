#pragma once

#include "Rendering/Pipelines/RenderPipeline.h"
#include <string>

struct VkContext;

class MSDFTextPipeline : public RenderPipeline
{
  public:
    MSDFTextPipeline(const VkContext& inContext) : RenderPipeline(inContext) {}
    void Initialize() override;

  private:
    const std::string vertexPath = "Assets/Engine/Shaders/UI";
    const std::string fragmentPath = "Assets/Engine/Shaders/MSDFText";
};