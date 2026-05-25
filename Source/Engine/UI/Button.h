#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

struct Button
{
    // Called when mouse button is released
    entt::sigh<void()> onClick{};

    template <auto Candidate, typename Type>
    void Connect(Type* instance)
    {
        entt::sink{ onClick }.template connect<Candidate>(instance);
    }

    template <auto Candidate, typename Type>
    void Disconnect(Type* instance)
    {
        entt::sink{ onClick }.template disconnect<Candidate>(instance);
    }
};

struct UIEventStyle
{
    glm::vec4 normal = glm::vec4(1.0f);
    glm::vec4 hover = glm::vec4(1.0f);
    glm::vec4 press = glm::vec4(1.0f);
};

enum UIState : uint8_t
{
    None = 0,
    Hovered = 1,
    Pressed = 2,
};

struct UIInteractionState
{
    UIState previous;
    UIState current;
};

struct UIClickCommand
{
    glm::vec2 position;
};