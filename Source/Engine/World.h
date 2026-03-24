#pragma once

#include "Registry/CommandBuffer.h"
#include <entt/entt.hpp>
#include <functional>

class Player;
class RenderingSystem;
class Registry;
class CommandBuffer;

class World
{
  public:
    bool Initialize(int argc, const char* argv[]);
    void Shutdown();

    void CreateWorld();

    void Run();
    void NetPulse();
    void RunSim(uint32_t tick);
    void Render(float alpha);

    void EndFrameCommandBuffer();

    void GCPass();

  private:
    World() = default;

    Registry* registry = nullptr;
    CommandBuffer globalCmd{};

    Player* player = nullptr;

    uint32_t simTick = 0;

    friend class Engine;
};