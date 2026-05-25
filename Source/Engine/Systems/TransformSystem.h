#pragma once

#include "Systems/ISystem.h"
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

class CommandBuffer;
class Registry;

class TransformSystem : public ISystem
{
  private:
    void Init(Registry* registry, CommandBuffer& cmd) override {}

    void Run(Registry* registry, CommandBuffer& cmd) override;
    void RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick) override;

    void UpdateTransformHierarchy(Registry* registry);
    void TransformToRenderTransform(Registry* registry);
};
