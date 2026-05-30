#pragma once

#include "Systems/ISystem.h"

struct LuaState;

class LuaSystem : public ISystem
{
  public:
    void Init(Registry* registry, CommandBuffer& cmd) override;
    void RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick) override;

  private:
    void SetGameState(LuaState& state);
    void SetUIState(LuaState& state);
    void HotReload();
};