#include "Engine.h"

#include "Callme/CallMe.h"
#include "InputManager.h"
#include "Rendering/RenderingSystem.h"
#include "SDLLayer.h"
#include "World.h"
#include <SDL3/SDL_timer.h>
#include <cstdint>

#if MAP_EDITOR
#include "MapWorld.h"
#endif

// TODO read from config what's the target FPS for the engine
constexpr int32_t TARGET_FPS = 30;

constexpr int32_t SIM_STEP = 1000000000 / TARGET_FPS;
constexpr int32_t NET_STEP = 100000000; // 100 ms

constexpr uint32_t SPIRAL_CHECK = 250000000; // 250ms

constexpr float GC_DELAY = 5.0f;

Engine* Engine::instance = nullptr;
Engine* Engine::Get()
{
    return instance;
}

Engine::~Engine()
{
    instance = nullptr;
}

Engine::Engine()
{
    instance = this;
}

#if MAP_EDITOR
static MapWorld mapWorld{};
#endif

bool Engine::Initialize(int argc, const char* argv[])
{
    isRunning = true;

    isRunning &= SDLLayer::Init();
    isRunning &= RenderingSystem::Get().Initialize();

#if !MAP_EDITOR
    defaultWorld = new World();
    isRunning &= defaultWorld->Initialize(argc, argv);
#else
    mapWorld.Init();
#endif

    InputManager::Get().onCloseGame = CallMe::Delegate<void()>(new auto([this]()
                                                                        { isRunning = false; }));

    uint64_t lastTime = SDL_GetTicksNS();
    uint64_t accumulator = 0;
    uint64_t netAccumulator = 0;

    float gcDelay = 0.0f;

    while (isRunning)
    {
        uint64_t currentTime = SDL_GetTicksNS();
        uint64_t frameTime = currentTime - lastTime;

        deltaTime = static_cast<double>(currentTime - lastTime) / 1000000000.0;

        if (frameTime > SPIRAL_CHECK)
            frameTime = SPIRAL_CHECK;

        lastTime = currentTime;

        accumulator += frameTime;
        netAccumulator += frameTime;

#if !MAP_EDITOR
        defaultWorld->Run();

        while (accumulator >= SIM_STEP)
        {
            defaultWorld->RunSim(tick++);
            accumulator -= SIM_STEP;
        }

        while (netAccumulator >= NET_STEP)
        {
            defaultWorld->NetPulse();
            netAccumulator -= NET_STEP;
        }

        float alpha = static_cast<float>(accumulator) / static_cast<float>(SIM_STEP);
        defaultWorld->Render(alpha);

        defaultWorld->EndFrameCommandBuffer();

        gcDelay += deltaTime;
        if (gcDelay >= GC_DELAY)
        {
            defaultWorld->GCPass();
            gcDelay = 0.0f;
        }
#else
        mapWorld.Run();
        mapWorld.Render();
        mapWorld.EndFrameCommandBuffer();
#endif
    }

    return true;
}

void Engine::Shutdown()
{
    SDLLayer::Shutdown();

#if !MAP_EDITOR
    defaultWorld->Shutdown();
    delete defaultWorld;
    defaultWorld = nullptr;
#else
#endif
}