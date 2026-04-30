#pragma once

class CommandBuffer;
class Registry;

class NavigationSystem
{
  public:
    NavigationSystem();

  private:
    void Init(Registry* registry, CommandBuffer& cmd);
    void Run(Registry* registry, CommandBuffer& cmd);
};