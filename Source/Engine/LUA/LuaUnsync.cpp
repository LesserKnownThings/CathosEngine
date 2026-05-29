#include "LuaUnsync.h"
#include "Debug/DebugSystem.h"
#include <entt/entt.hpp>
#include <format>
#include <lua.hpp>
#include <string>

constexpr std::string LUA_UNSYNC_LOG = "Lua_Unsync";

void LuaUnsync::DispatchClick(lua_State* l, entt::entity pressedEntity)
{
    lua_getglobal(l, "UI");

    if (!lua_istable(l, -1))
    {
        LOG(LUA_UNSYNC_LOG, Error, "Global UI table not found!");
        return;
    }

    lua_getfield(l, -1, "DispatchClick");
    if (!lua_isfunction(l, -1))
    {
        LOG(LUA_UNSYNC_LOG, Error, "UI.DispatchClick is not a function!");
        return;
    }

    lua_remove(l, -2);

    auto entityRawId = static_cast<lua_Integer>(pressedEntity);
    lua_pushinteger(l, entityRawId);

    if (lua_pcall(l, 1, 0, 0) != LUA_OK)
    {
        const char* error = lua_tostring(l, -1);
        LOG(LUA_UNSYNC_LOG, Error, std::format("Lua Error in UI Dispatcher: {}", error));

        lua_pop(l, 1);
    }
}