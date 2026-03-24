#pragma once

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

class CommandBuffer;
class Registry;

class TransformSystem
{
  public:
    TransformSystem();

  private:
    void Run(Registry* registry, CommandBuffer& cmd);
    void RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick);

    void UpdateTransformHierarchy(Registry* registry);
    void SyncSimTransformToRenderTransform(Registry* registry);
};
