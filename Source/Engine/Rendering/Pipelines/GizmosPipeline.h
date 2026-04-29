#pragma once

#include "RenderPipeline.h"

#include <string>

struct VkContext;

class GizmosPipeline : public RenderPipeline
{
  public:
    GizmosPipeline(const VkContext& inContext);

    EPipelineType GetType() const override { return EPipelineType::Gizmos; }

  private:
    const std::string shaderPath = "Data/Engine/Shaders/Gizmos";
};