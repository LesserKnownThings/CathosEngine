#pragma once

#include <cstdint>

class Registry;
class CommandBuffer;

class ISystem
{
  public:
    virtual ~ISystem() = default;

    virtual void Init(Registry* registry, CommandBuffer& cmd) = 0;

    virtual void Run(Registry* registry, CommandBuffer& cmd) {}
    virtual void RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick) {}
};