#pragma once

#include <cstdint>
class World;

#define GEngine = Engine::Get()

class Engine
{

  public:
    ~Engine();
    Engine();

    static Engine* Get();

    bool Initialize(int argc, const char* argv[]);
    void Shutdown();

  private:
    World* defaultWorld = nullptr;

    bool isRunning = false;
    uint32_t tick = 0;

    double deltaTime = 0.0;

    static Engine* instance;
};