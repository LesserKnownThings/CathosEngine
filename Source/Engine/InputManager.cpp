#include "InputManager.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <glm/common.hpp>
#include <glm/geometric.hpp>

#if EDITOR
#include <backends/imgui_impl_sdl3.h>
#endif

InputManager& InputManager::Get()
{
    static InputManager instance{};
    return instance;
}

InputManager::InputManager()
{
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
#if EDITOR
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

        if (e.key.down)
        {
        }
        else
        {
            if (e.key.scancode == SDL_SCANCODE_F5)
            {
                onHotReload.raise();
            }
        }

        if (e.type == SDL_EVENT_MOUSE_MOTION)
        {
            SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
            onMouseMoved.raise(mousePosition);
        }

        if (e.button.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            ProcessButton(e.button.button, MOUSE_JUST_PRESSED_STATE);
            ProcessButton(e.button.button, MOUSE_DOWN_STATE);

            if ((cachedMouseButton & e.button.button) == 0)
            {
                cachedMouseButton |= e.button.button;
                onMouseClicked.raise(e.button.button);
            }
        }
        else if (e.button.type == SDL_EVENT_MOUSE_BUTTON_UP)
        {
            ProcessButton(e.button.button, MOUSE_JUST_RELEASED_STATE);

            onMouseReleased.raise(e.button.button);
            cachedMouseButton &= ~e.button.button;
        }
    }

    ProcessKeys();
}

void InputManager::FlushInput()
{
    for (int32_t i = 0; i < mouseState.size(); ++i)
    {
        const uint32_t combined = mouseState[i];

        uint16_t state = combined & 0xFFFF;
        uint8_t button = (combined >> 16) & 0xFF;

        if ((state & MOUSE_JUST_RELEASED_STATE) != 0)
        {
            state &= ~MOUSE_DOWN_STATE;
        }

        state &= ~(MOUSE_JUST_PRESSED_STATE | MOUSE_JUST_RELEASED_STATE);

        const uint32_t newCombined = state | (static_cast<uint32_t>(button) << 16);

        if ((state & MOUSE_DOWN_STATE) == 0)
        {
            mouseState.erase(mouseState.begin() + i);
            --i;
        }
        else
        {
            mouseState[i] = newCombined;
        }
    }
}

void InputManager::ProcessButton(uint8_t button, uint32_t state)
{
    auto it = std::find_if(mouseState.begin(), mouseState.end(), [&button](uint32_t iterator)
                           { 
                                uint8_t currentButton = (iterator >> 16) & 0xFF;
                                return button == currentButton; });

    if (it != mouseState.end())
    {
        const int32_t index = std::distance(mouseState.begin(), it);

        const uint32_t combined = *it;

        uint16_t currentState = combined & 0xFFFF;

        currentState |= state;

        mouseState[index] = currentState | (static_cast<uint32_t>(button) << 16);
    }
    else
    {
        const uint32_t combined = state | (static_cast<uint32_t>(button) << 16);
        mouseState.push_back(combined);
    }
}

void InputManager::ProcessKeys()
{
    // movementAxis.y = storedKeys[SDLK_W] + storedKeys[SDLK_S] * -1.0f;
    // movementAxis.x = storedKeys[SDLK_D] + storedKeys[SDLK_A] * -1.0f;

    int numKeys;
    const bool* keyState = SDL_GetKeyboardState(&numKeys);

    // TODO move the mouse to a different function
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

bool InputManager::IsMouseButtonJustPressed(uint8_t button) const
{
    for (uint32_t currentMouseState : mouseState)
    {
        const uint16_t states = currentMouseState & 0xFFFF;
        const uint8_t currentButton = (currentMouseState >> 16) & 0xFF;

        if (currentButton == button && (states & MOUSE_JUST_PRESSED_STATE))
        {
            return true;
        }
    }

    return false;
}

bool InputManager::IsMouseButtonJustReleased(uint8_t button) const
{
    for (uint32_t currentMouseState : mouseState)
    {
        const uint16_t states = currentMouseState & 0xFFFF;
        const uint8_t currentButton = (currentMouseState >> 16) & 0xFF;

        if (currentButton == button && (states & MOUSE_JUST_RELEASED_STATE))
        {
            return true;
        }
    }

    return false;
}

bool InputManager::IsMouseButtonDown(uint8_t button) const
{
    for (uint32_t currentMouseState : mouseState)
    {
        const uint16_t states = currentMouseState & 0xFFFF;
        const uint8_t currentButton = (currentMouseState >> 16) & 0xFF;

        if (currentButton == button && (states & MOUSE_DOWN_STATE))
        {
            return true;
        }
    }

    return false;
}