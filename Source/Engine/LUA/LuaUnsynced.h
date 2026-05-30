#pragma once

#include <entt/fwd.hpp>

struct lua_State;

class LuaUnsynced
{
  public:
    static void DispatchClick(lua_State* l, entt::entity pressedEntity);
};