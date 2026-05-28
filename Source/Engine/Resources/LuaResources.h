#pragma once

#include <cstdint>
#include <lua.hpp>
#include <vector>

struct LuaState
{
    // UI state
    lua_State* unsyncState;
    // Game state
    lua_State* syncState;
};

// Used to track user defined data
struct LuaScriptData
{
    std::vector<int32_t> ints;
    std::vector<float> flots;
};
