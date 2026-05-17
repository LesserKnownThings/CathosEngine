#pragma once

class CommandBuffer;
class Registry;

class UISystem
{
  public:
    UISystem();

  private:
    void Init(Registry* registry, CommandBuffer& cmd);
};