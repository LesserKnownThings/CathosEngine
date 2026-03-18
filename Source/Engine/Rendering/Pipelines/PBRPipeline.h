#pragma once

#include "RenderPipeline.h"

#include <string>

struct VkContext;

class PBRPipeline : public RenderPipeline
{
  public:
    PBRPipeline(const VkContext& inContext);

    EPipelineType GetType() const override { return EPipelineType::PBR; }

  private:
    const std::string shaderPath = "Data/Engine/Shaders/PBR";
};