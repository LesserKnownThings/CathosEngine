#pragma once

#include "Systems/ISystem.h"
#include <cstdint>

class NavigationSystem : ISystem
{
  private:
    void Init(Registry* registry, CommandBuffer& cmd) override;
    void RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick) override;
};