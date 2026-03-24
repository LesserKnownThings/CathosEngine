#pragma once

#include <cstdint>

class CommandBuffer;
class Registry;

class SpatialPartitionSystem
{
  public:
    SpatialPartitionSystem();

  private:
    void Init(Registry* registry, CommandBuffer& cmd);
    void RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick);
};