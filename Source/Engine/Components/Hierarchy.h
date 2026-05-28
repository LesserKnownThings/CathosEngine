#pragma once

#include <entt/entt.hpp>

struct ChildOf
{
    entt::entity entity = entt::null;
};

struct Children
{
    std::vector<entt::entity> children;
};