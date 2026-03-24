#include "InputManager.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <glm/common.hpp>

#if MAP_EDITOR
#include <backends/imgui_impl_sdl3.h>
#endif

InputManager& InputManager::Get()
{
    static InputManager instance{};
    return instance;
}

InputManager::InputManager()
{
    storedKeys.resize(SDL_SCANCODE_COUNT);

    // TODO enable this once I have custom cursor rendering
    //  SDL_SetWindowRelativeMouseMode(RenderingSystem::Get().GetWindow(), true);
}

void InputManager::ShowMouse()
{
    SDL_ShowCursor();
}

void InputManager::HideMouse()
{
    SDL_HideCursor();
}

void InputManager::PollInput()
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
#if MAP_EDITOR
        ImGui_ImplSDL3_ProcessEvent(&e);
#endif

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
        else
        {
            if (e.key.down)
            {
                if (e.key.repeat == 0 && e.key.key < SDL_SCANCODE_COUNT)
                {
                    storedKeys[e.key.key] = 1;
                }
            }
            else if (!e.key.down && e.key.key < SDL_SCANCODE_COUNT)
            {
                storedKeys[e.key.key] = 0;
            }
        }

        if (e.type == SDL_EVENT_MOUSE_MOTION)
        {
            SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
            onMouseMoved.raise(mousePosition);
        }

        if (e.button.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            if ((cachedMouseButton & e.button.button) == 0)
            {
                cachedMouseButton |= e.button.button;
                onMouseClicked.raise(e.button.button);
            }
        }
        else if (e.button.type == SDL_EVENT_MOUSE_BUTTON_UP)
        {
            onMouseReleased.raise(e.button.button);
            cachedMouseButton &= ~e.button.button;
        }
    }

    ProcessKeys();
}

void InputManager::ProcessKeys()
{
    movementAxis.y = storedKeys[SDLK_W] + storedKeys[SDLK_S] * -1.0f;
    movementAxis.x = storedKeys[SDLK_D] + storedKeys[SDLK_A] * -1.0f;

    float deltaX, deltaY;
    SDL_GetRelativeMouseState(&deltaX, &deltaY);

    constexpr float MIN_ACCUMULATED_DELTA = 3.0f;

    mouseDelta = glm::vec2(0.0f);
    if (std::abs(deltaX) >= MIN_ACCUMULATED_DELTA)
    {
        mouseDelta.x = glm::clamp(deltaX, -1.0f, 1.0f);
    }

    if (std::abs(deltaY) >= MIN_ACCUMULATED_DELTA)
    {
        mouseDelta.y = glm::clamp(deltaY, -1.0f, 1.0f);
    }

    SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
}