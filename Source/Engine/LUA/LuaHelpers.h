#pragma once

#include <glm/glm.hpp>
#include <lua.hpp>

inline void PushVec4(lua_State* l, const char* key, const glm::vec4& vec)
{
    lua_createtable(l, 4, 0);

    lua_pushnumber(l, vec.x);
    lua_rawseti(l, -2, 1);

    lua_pushnumber(l, vec.y);
    lua_rawseti(l, -2, 2);

    lua_pushnumber(l, vec.z);
    lua_rawseti(l, -2, 3);

    lua_pushnumber(l, vec.w);
    lua_rawseti(l, -2, 4);
}

inline void ParseVec2(lua_State* l, int32_t tableIdx, const char* key, glm::vec2& outVec)
{
    lua_getfield(l, tableIdx, key);
    if (lua_istable(l, -1))
    {
        lua_rawgeti(l, -1, 1);
        outVec.x = static_cast<float>(lua_tonumber(l, -1));
        lua_pop(l, 1);

        lua_rawgeti(l, -1, 2);
        outVec.y = static_cast<float>(lua_tonumber(l, -1));
        lua_pop(l, 1);
    }
    lua_pop(l, 1);
};

inline void ParseVec4(lua_State* l, int32_t tableIdx, const char* key, glm::vec4& outVec)
{
    lua_getfield(l, tableIdx, key);
    if (lua_istable(l, -1))
    {
        lua_rawgeti(l, -1, 1);
        outVec.r = static_cast<float>(lua_tonumber(l, -1));
        lua_pop(l, 1);
        lua_rawgeti(l, -1, 2);
        outVec.g = static_cast<float>(lua_tonumber(l, -1));
        lua_pop(l, 1);
        lua_rawgeti(l, -1, 3);
        outVec.b = static_cast<float>(lua_tonumber(l, -1));
        lua_pop(l, 1);
        lua_rawgeti(l, -1, 4);
        outVec.a = static_cast<float>(lua_tonumber(l, -1));
        lua_pop(l, 1);
    }
    lua_pop(l, 1);
};