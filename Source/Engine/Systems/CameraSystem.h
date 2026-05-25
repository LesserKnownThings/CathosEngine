#pragma once

#include "Systems/ISystem.h"

class CommandBuffer;
class Registry;

class CameraSystem : public ISystem
{
  private:
    void Init(Registry* registry, CommandBuffer& cmd) override;
    void Run(Registry* registry, CommandBuffer& cmd) override;
};