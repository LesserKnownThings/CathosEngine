#include "LuaSystem.h"
#include "Debug/DebugSystem.h"
#include "InputManager.h"
#include "LUA/LuaSyncedState.h"
#include "LUA/LuaUnsyncedState.h"
#include "Registry/CommandBuffer.h"
#include "Registry/Registry.h"
#include "Resources/LuaResources.h"
#include "Systems/SystemRegistry.h"
#include "World.h"

#include <filesystem>
#include <format>
#include <lauxlib.h>
#include <lua.h>
#include <lua.hpp>

REGISTER_SYSTEM(LuaSystem, SystemPhase::Simulation, DEPENDENCIES({}), DEPENDENCIES({}), 0);

constexpr std::string LUA_LOG = "LUA";

void LuaSystem::Init(Registry* registry, CommandBuffer& cmd)
{
    static auto temp = InputManager::Get().onHotReload.subscribe(CallMe::fromMethod<&LuaSystem::HotReload>(this));

    registry->AddResource<LuaScriptData>();
    LuaState& luaState = registry->AddResource<LuaState>();
    SetGameState(luaState);
    SetUIState(luaState);
}

void LuaSystem::RunSync(Registry* registry, CommandBuffer& cmd, uint32_t tick)
{
    const LuaState& state = registry->GetResource<LuaState>();

    lua_getglobal(state.syncedState, "GM");
}

void LuaSystem::HotReload()
{
    Registry* reg = World::GetRegistry();
    LuaState& luaState = reg->AddResource<LuaState>();

    auto unsynced = luaState.unsyncedState;

    lua_getglobal(unsynced, "UIManager");
    lua_getfield(unsynced, -1, "Deinit");
    lua_pushvalue(unsynced, -2);

    if (lua_pcall(unsynced, 1, 0, 0) != LUA_OK)
    {
        LOG(LUA_LOG, Error, std::format("UI UIManager:Deinit Error: {}", lua_tostring(unsynced, -1)));
        lua_pop(unsynced, 1);
    }

    lua_getfield(unsynced, -1, "Init");
    lua_pushvalue(unsynced, -2);

    if (lua_pcall(unsynced, 1, 0, 0) != LUA_OK)
    {
        LOG(LUA_LOG, Error, std::format("UI UIManager:Init Error: {}", lua_tostring(unsynced, -1)));
        lua_pop(unsynced, 1);
    }

    lua_pop(unsynced, 1);
}

void LuaSystem::SetGameState(LuaState& state)
{
    auto& l = state.syncedState;

    l = luaL_newstate();
    luaL_openlibs(l);
    // Prevent scripts from messing with the OS directly
    lua_getglobal(l, "os");
    lua_pushnil(l);
    // Delete the os.execute so we cant run terminal commands
    lua_setfield(l, -2, "execute");
    lua_pushnil(l);
    // Delete os.exit so we can't close the game from Lua
    lua_setfield(l, -2, "exit");
    lua_pop(l, 1);

    // Delete package.loadlib to prevent loading of libs or dlls
    lua_getglobal(l, "package");
    lua_pushnil(l);
    lua_setfield(l, -2, "loadlib");
    lua_pop(l, 1);

    // GM
    lua_newtable(l);
    // VFS
    lua_newtable(l);

    lua_pushcfunction(l, l_GetFilesInFolderRecursive);
    lua_setfield(l, -2, "GetFilesRecursive");

    lua_pushcfunction(l, l_GetFile);
    lua_setfield(l, -2, "GetFile");

    lua_setfield(l, -2, "VFS");

    lua_setglobal(l, "GM");
}

void LuaSystem::SetUIState(LuaState& state)
{
    auto& unsynced = state.unsyncedState;

    unsynced = luaL_newstate();
    luaL_openlibs(unsynced);
    // Prevent scripts from messing with the OS directly
    lua_getglobal(unsynced, "os");
    lua_pushnil(unsynced);
    // Delete the os.execute so we cant run terminal commands
    lua_setfield(unsynced, -2, "execute");
    lua_pushnil(unsynced);
    // Delete os.exit so we can't close the game from Lua
    lua_setfield(unsynced, -2, "exit");
    lua_pop(unsynced, 1);

    // Delete package.loadlib to prevent loading of libs or dlls
    lua_getglobal(unsynced, "package");
    lua_pushnil(unsynced);
    lua_setfield(unsynced, -2, "loadlib");
    lua_pop(unsynced, 1);

    // Creating the UI proxy table
    lua_newtable(unsynced);

    // Creating the FileSystem sub table
    lua_newtable(unsynced);

    lua_pushcfunction(unsynced, l_GetUIFilesInFolderRecursive);
    lua_setfield(unsynced, -2, "GetFilesRecursive");

    lua_pushcfunction(unsynced, l_GetUIFile);
    lua_setfield(unsynced, -2, "GetFile");

    lua_setfield(unsynced, -2, "VFS");

    lua_pushcfunction(unsynced, l_CreateWidget);
    lua_setfield(unsynced, -2, "CreateWidget");

    lua_pushcfunction(unsynced, l_SetParent);
    lua_setfield(unsynced, -2, "SetParent");

    lua_pushcfunction(unsynced, l_SetLayout);
    lua_setfield(unsynced, -2, "SetLayout");

    lua_pushcfunction(unsynced, l_SetVisibility);
    lua_setfield(unsynced, -2, "SetVisibility");

    lua_pushcfunction(unsynced, l_CreateImage);
    lua_setfield(unsynced, -2, "CreateImage");

    lua_pushcfunction(unsynced, l_AddNineSliceComponent);
    lua_setfield(unsynced, -2, "AddNineSlice");

    lua_pushcfunction(unsynced, l_DestroyEntity);
    lua_setfield(unsynced, -2, "DestroyEntity");

    lua_pushcfunction(unsynced, l_CreateText);
    lua_setfield(unsynced, -2, "CreateText");

    lua_pushcfunction(unsynced, l_SetTextStyle);
    lua_setfield(unsynced, -2, "SetTextStyle");

    lua_pushcfunction(unsynced, l_CreateHBox);
    lua_setfield(unsynced, -2, "CreateHBox");

    lua_pushcfunction(unsynced, l_CreateVBox);
    lua_setfield(unsynced, -2, "CreateVBox");

    lua_pushcfunction(unsynced, l_CreateButton);
    lua_setfield(unsynced, -2, "CreateButton");

    lua_pushcfunction(unsynced, l_SendUIMessage);
    lua_setfield(unsynced, -2, "SendUIMessage");

    lua_setglobal(unsynced, "UI");

    std::filesystem::path uiRouter = "LuaUI/ui_main.lua";
    std::filesystem::path uiDefinitions = "LuaUI/ui_definitions.lua";
    std::string gameDefinitions = "LuaGame/definitions.lua";

    if (luaL_dofile(unsynced, uiRouter.string().c_str()) != LUA_OK)
    {
        LOG(LUA_LOG, Error, std::format("UI Framework Boot Error: {}", lua_tostring(unsynced, -1)));
        lua_pop(unsynced, 1);
    }
    else
    {
        luaL_dofile(unsynced, uiDefinitions.string().c_str());
        luaL_dofile(unsynced, gameDefinitions.c_str());

        lua_getglobal(unsynced, "UIManager");
        lua_getfield(unsynced, -1, "Init");
        lua_pushvalue(unsynced, -2);

        if (lua_pcall(unsynced, 1, 0, 0) != LUA_OK)
        {
            LOG(LUA_LOG, Error, std::format("UI UIManager:Init Error: {}", lua_tostring(unsynced, -1)));
            lua_pop(unsynced, 1);
        }
    }
}
