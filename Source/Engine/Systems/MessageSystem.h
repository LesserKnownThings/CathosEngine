#pragma once

#include "Systems/ISystem.h"

class Registry;
class CommandBuffer;

class MessageSystem : public ISystem
{
  public:
    void Init(Registry* registry, CommandBuffer& cmd) override;

    void Run(Registry* registry, CommandBuffer& cmd) override;
    void RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick) override;
};