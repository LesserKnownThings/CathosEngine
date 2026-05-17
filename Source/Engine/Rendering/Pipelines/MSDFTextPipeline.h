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
    const std::string vertexPath = "Data/Engine/Shaders/UI";
    const std::string fragmentPath = "Data/Engine/Shaders/MSDFText";
};