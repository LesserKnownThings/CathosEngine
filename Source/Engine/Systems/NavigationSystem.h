#pragma once

#include <cstdint>

class CommandBuffer;
class Registry;

class NavigationSystem
{
  public:
    NavigationSystem();

  private:
    void Init(Registry* registry, CommandBuffer& cmd);
    void RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick);
};