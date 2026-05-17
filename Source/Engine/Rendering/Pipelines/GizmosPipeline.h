#pragma once

#include "RenderPipeline.h"

#include <string>

struct VkContext;

class GizmosPipeline : public RenderPipeline
{
  public:
    GizmosPipeline(const VkContext& inContext) : RenderPipeline(inContext) {}
    void Initialize() override;

  private:
    const std::string shaderPath = "Data/Engine/Shaders/Gizmos";
};