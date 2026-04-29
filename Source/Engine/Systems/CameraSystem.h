#pragma once

class CommandBuffer;
class Registry;

class CameraSystem
{
  public:
    CameraSystem();

  private:
    void Init(Registry* registry, CommandBuffer& cmd);
    void Run(Registry* registry, CommandBuffer& cmd);
};