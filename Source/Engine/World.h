#pragma once

#include "Registry/CommandBuffer.h"
#include <entt/entt.hpp>

class Player;
class RenderingSystem;
class Registry;
class CommandBuffer;

class World
{
  public:
    bool Initialize(int argc, const char* argv[]);
    void Shutdown();

    void CreateWorld(CommandBuffer& cmd);

    void Run();
    void NetPulse();
    void RunSim(uint32_t tick);
    void Render(float alpha);

    void GCPass();

  private:
    void Test();

    World() = default;

    Registry* registry = nullptr;
    Player* player = nullptr;

    uint32_t simTick = 0;

    friend class Engine;
};