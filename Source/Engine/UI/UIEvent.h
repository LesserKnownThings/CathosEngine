#pragma once

#include <entt/entt.hpp>

struct UIEvent
{
    entt::entity hovered = entt::null;
    entt::entity pressed = entt::null;
};