#pragma once

#include "entt/entt.hpp"
#include <entt/entity/fwd.hpp>

class Player;
class RenderingSystem;

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

    void GCPass();

  private:
    World() = default;

    void UpdateTransformHierarchy(const entt::entity* storage, auto& group);
    void SyncSimTransformToRenderTransform();

    entt::registry registry;

    Player* player = nullptr;

    uint32_t currentTick = 0;

    friend class Engine;
};