#include "Engine.h"

#include "Callme/CallMe.h"
#include "InputManager.h"
#include "Rendering/RenderingSystem.h"
#include "SDLLayer.h"
#include "World.h"
#include <SDL3/SDL_timer.h>
#include <cstdint>

#if EDITOR
#include "EditorWorld.h"
#endif

// TODO read from config what's the target FPS for the engine
constexpr uint64_t TARGET_FPS = 30;
constexpr uint64_t SIM_STEP = 1000000000ull / TARGET_FPS;
constexpr uint64_t NET_DIVISOR = 3; // For 10 hz

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

#if EDITOR
static EditorWorld editorWorld{};
#endif

bool Engine::Initialize(int argc, const char* argv[])
{
    isRunning = true;

    isRunning &= SDLLayer::Init();
    isRunning &= RenderingSystem::Get().Initialize();

#if !EDITOR
    defaultWorld = new World();
    isRunning &= defaultWorld->Initialize(argc, argv);
#else
    editorWorld.Init();
#endif

    InputManager& im = InputManager::Get();
    im.onCloseGame = CallMe::Delegate<void()>(new auto([this]()
                                                       { isRunning = false; }));

    uint64_t lastTime = SDL_GetTicksNS();
    uint64_t accumulator = 0;

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

        im.PollInput();

#if !EDITOR
        defaultWorld->FrameStart();
        defaultWorld->Run();

        while (accumulator >= SIM_STEP)
        {
            defaultWorld->RunSim(tick);

            if (tick % NET_DIVISOR == 0)
            {
                defaultWorld->NetPulse();
            }

            tick++;
            accumulator -= SIM_STEP;
        }

        float alpha = static_cast<float>(accumulator) / static_cast<float>(SIM_STEP);
        defaultWorld->Render(alpha);
        defaultWorld->FrameEnd();

        gcDelay += deltaTime;
        if (gcDelay >= GC_DELAY)
        {
            defaultWorld->GCPass();
            gcDelay = 0.0f;
        }
#else
        editorWorld.Run();
        editorWorld.Render();
        editorWorld.EndFrameCommandBuffer();
#endif

        im.FlushInput();
    }

    return true;
}

void Engine::Shutdown()
{
    SDLLayer::Shutdown();

#if !EDITOR
    defaultWorld->Shutdown();
    delete defaultWorld;
    defaultWorld = nullptr;
#else
#endif
}