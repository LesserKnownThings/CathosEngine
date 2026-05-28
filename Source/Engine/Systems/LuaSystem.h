#pragma once

#include "Systems/ISystem.h"

class LuaSystem : public ISystem
{
  public:
    void Init(Registry* registry, CommandBuffer& cmd) override;

  private:
    void HotReload();
};