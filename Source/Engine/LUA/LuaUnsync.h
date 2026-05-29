#pragma once

#include <entt/fwd.hpp>

struct lua_State;

class LuaUnsync
{
  public:
    static void DispatchClick(lua_State* l, entt::entity pressedEntity);
};