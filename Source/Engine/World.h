#pragma once

#include "Registry/CommandBuffer.h"
#include <entt/entt.hpp>

class Player;
class RenderingSystem;
class Registry;

class World
{
  public:
    bool Initialize(int argc, const char* argv[]);
    void Shutdown();

    void CreateWorld(CommandBuffer& cmd);

    /**
     * These 2 functions are made for the global command frame buffer
     */
    // This is called at the start of the frame before all systems, it's not sync / unsync specific
    void FrameStart();
    // This is the same as the FrameInit, but it's called at the end of the frame after all systems
    void FrameEnd();

    void Run();
    void NetPulse();
    void RunSim(uint32_t tick);
    void Render(float alpha);

    void GCPass();

    static Registry* GetRegistry() { return registry; }
    static AssetServer& GetAssetServer() { return registry->GetAssetServer(); }
    static CommandBuffer& GetFrameStartCommandBuffer() { return frameStartCommandBuffer; }
    static CommandBuffer& GetFrameEndCommandBuffer() { return frameEndCommandBuffer; }

  private:
    void Test();

    World() = default;

    Player* player = nullptr;

    uint32_t simTick = 0;

    static Registry* registry;

    // Will run all its commands at the start of the frame
    static CommandBuffer frameStartCommandBuffer;
    // Will run all its commands at the end of the frame
    static CommandBuffer frameEndCommandBuffer;

    friend class Engine;
};