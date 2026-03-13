#include "InputManager.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

InputManager& InputManager::Get()
{
    static InputManager instance{};
    return instance;
}

void InputManager::PollInput(uint32_t tick)
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_EVENT_QUIT)
        {
            onCloseGame.invoke();
        }
        else if (e.type == SDL_EVENT_WINDOW_RESIZED)
        {
            onWindowResize.raise();
        }
        else if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
        {
            onWindowMinimized.raise();
        }
    }
}