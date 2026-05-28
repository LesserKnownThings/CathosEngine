#include "LuaSystem.h"
#include "Components/Hierarchy.h"
#include "Debug/DebugSystem.h"
#include "InputManager.h"
#include "Registry/CommandBuffer.h"
#include "Registry/Registry.h"
#include "Resources/AssetPath.h"
#include "Resources/AssetServer.h"
#include "Resources/Font.h"
#include "Resources/LuaResources.h"
#include "Resources/Texture.h"
#include "Systems/SystemRegistry.h"
#include "UI/LayoutBox.h"
#include "UI/NineSlice.h"
#include "UI/TextRenderer.h"
#include "UI/UIMaterial.h"
#include "UI/UITransform.h"
#include "UI/UIVisibility.h"
#include "World.h"
#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <filesystem>
#include <format>
#include <lauxlib.h>
#include <lua.h>
#include <lua.hpp>

REGISTER_SYSTEM(LuaSystem, SystemPhase::Simulation, DEPENDENCIES({}), DEPENDENCIES({}), 0);

constexpr std::string LUA_LOG = "LUA";

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

int32_t l_ExecuteUIScript(lua_State* l)
{
    const char* relativePath = luaL_checkstring(l, 1);

    std::filesystem::path assetRoot = "LuaUI/Widgets";
    std::filesystem::path fullPath = assetRoot / relativePath;

    if (!std::filesystem::exists(fullPath))
    {
        std::string errorMsg = "[C++ FileSystem Error] File not found: " + fullPath.string();
        lua_pushstring(l, errorMsg.c_str());
        return lua_error(l);
    }

    if (luaL_dofile(l, fullPath.string().c_str()) != LUA_OK)
    {
        return lua_error(l);
    }

    return 0;
}

// Should probably rename this to empty container?
int32_t l_CreateWidget(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();

    entt::entity instance = reg.create();
    reg.emplace<UITransform>(instance);

    auto entityId = static_cast<lua_Integer>(instance);
    lua_pushinteger(l, entityId);

    return 1;
}

// Should probably rename this to destroy widget?
int32_t l_DestroyEntity(lua_State* l)
{
    CommandBuffer& cmd = World::GetFrameEndCommandBuffer();

    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(l, 1));
    auto entity = static_cast<entt::entity>(entityId);

    cmd.Destroy(entity);

    return 0;
}

int32_t l_CreateImage(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();
    AssetServer& as = World::GetAssetServer();

    entt::entity instance = reg.create();
    reg.emplace<UIVisiblity>(instance);
    reg.emplace<UITransform>(instance, UITransform{
                                           .size = glm::vec2(100.f) });

    const char* optionalTexture = luaL_optstring(l, 1, nullptr);
    std::string texturePath = optionalTexture == nullptr ? "Assets/Engine/Textures/base_albedo.png" : optionalTexture;

    entt::resource<Texture2D> baseAbledo = as.Load<Texture2D>(AssetPath{ texturePath });
    reg.emplace<UIMaterial>(instance, baseAbledo);

    auto entityId = static_cast<lua_Integer>(instance);
    lua_pushinteger(l, entityId);

    return 1;
}

int32_t l_AddNineSliceComponent(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();

    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(l, 1));
    auto entity = static_cast<entt::entity>(entityId);

    NineSlice& ns = reg.emplace<NineSlice>(entity);

    if (!lua_istable(l, 2))
    {
        luaL_error(l, "Expected a table as the 2nd argument");
    }

    lua_getfield(l, 2, "left");
    ns.left = static_cast<float>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, 2, "right");
    ns.right = static_cast<float>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, 2, "top");
    ns.top = static_cast<float>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, 2, "bottom");
    ns.bottom = static_cast<float>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    return 0;
}

int32_t l_SetParent(lua_State* l)
{
    entt::registry& registry = World::GetRegistry()->Get();

    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(l, 1));
    auto entity = static_cast<entt::entity>(entityId);

    uint32_t parentId = static_cast<uint32_t>(luaL_checkinteger(l, 2));
    auto parentEntity = static_cast<entt::entity>(parentId);

    const auto& parentStorage = registry.storage<ChildOf>();
    const auto& childrenStorage = registry.storage<Children>();

    if (!parentStorage.contains(entity))
    {
        registry.emplace<ChildOf>(entity, parentEntity);
    }
    else
    {
        ChildOf& parent = registry.get<ChildOf>(entity);
        parent.entity = parentEntity;
    }

    if (!childrenStorage.contains(parentEntity))
    {
        registry.emplace<Children>(parentEntity, std::vector<entt::entity>{ entity });
    }
    else
    {
        Children& children = registry.get<Children>(parentEntity);

        bool exists = false;
        for (auto child : children.children)
        {
            if (child == entity)
            {
                exists = true;
            }
        }

        if (!exists)
        {
            children.children.push_back(entity);
        }
    }

    return 0;
}

int32_t l_SetLayout(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();

    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(l, 1));
    auto entity = static_cast<entt::entity>(entityId);

    if (!lua_istable(l, 2))
    {
        luaL_error(l, "Expected a table as the 2nd argument");
    }

    UITransform& transform = reg.get<UITransform>(entity);

    ParseVec2(l, 2, "position", transform.position);
    ParseVec2(l, 2, "size", transform.size);
    ParseVec2(l, 2, "anchorMin", transform.anchorMin);
    ParseVec2(l, 2, "anchorMax", transform.anchorMax);
    ParseVec2(l, 2, "pivot", transform.pivot);

    lua_getfield(l, 2, "localZ");
    transform.localZOrder = static_cast<int32_t>(lua_tointeger(l, -1));
    lua_pop(l, 1);

    return 0;
}

int32_t l_SetVisibility(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();

    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(l, 1));
    auto entity = static_cast<entt::entity>(entityId);

    UIVisiblity& visibility = reg.get<UIVisiblity>(entity);
    visibility.mode = static_cast<VisibilityMode>(lua_tointeger(l, 2));

    return 0;
}

int32_t l_CreateText(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();
    AssetServer& as = World::GetAssetServer();

    entt::entity instance = reg.create();

    reg.emplace<UIVisiblity>(instance);
    reg.emplace<UITransform>(instance);
    reg.emplace<TextStyle>(instance);

    if (!lua_istable(l, 1))
    {
        luaL_error(l, "Expected a table as the first argument");
    }

    TextRenderer& tr = reg.emplace<TextRenderer>(instance);

    lua_getfield(l, 1, "fontName");
    const std::string fontName = lua_tostring(l, -1);
    lua_pop(l, 1);

    lua_getfield(l, 1, "text");
    tr.text = lua_tostring(l, -1);
    lua_pop(l, 1);

    ParseVec4(l, 1, "color", tr.color);

    lua_getfield(l, 1, "fontSize");
    tr.fontSize = static_cast<float>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    tr.font = as.Load<Font>(AssetPath{ "LuaUI/Fonts/" + fontName });

    auto entityId = static_cast<lua_Integer>(instance);
    lua_pushinteger(l, entityId);

    return 1;
}

int32_t l_SetTextStyle(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();

    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(l, 1));
    auto entity = static_cast<entt::entity>(entityId);

    luaL_checktype(l, 2, LUA_TTABLE);

    TextStyle& style = reg.get<TextStyle>(entity);

    lua_getfield(l, 2, "charSpacing"); // Pushes value to top (-1)
    style.characterSpacing = static_cast<float>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, 2, "lineSpacing");
    style.lineSpacing = static_cast<float>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, 2, "horizontal");
    style.horizontal = static_cast<TextHAlign>(lua_tointeger(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, 2, "vertical");
    style.vertical = static_cast<TextVAlign>(lua_tointeger(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, 2, "wrapText");
    style.wrapText = lua_toboolean(l, -1);
    lua_pop(l, 1);

    return 0;
}

int32_t l_CreateHBox(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();

    if (!lua_istable(l, 1))
    {
        luaL_error(l, "Expected a table as the first argument");
    }

    auto instance = reg.create();
    reg.emplace<UITransform>(instance);

    HBox& box = reg.emplace<HBox>(instance);

    ParseVec4(l, 1, "offset", box.offset);

    lua_getfield(l, 1, "spacing");
    box.spacing = static_cast<float>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, 1, "childStart");
    box.childStart = static_cast<ChildStart>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, 1, "controlHSize");
    box.controlHSize = lua_toboolean(l, -1);
    lua_pop(l, 1);

    lua_getfield(l, 1, "controlVSize");
    box.controlVSize = lua_toboolean(l, -1);
    lua_pop(l, 1);

    auto entityId = static_cast<lua_Integer>(instance);
    lua_pushinteger(l, entityId);

    return 1;
}

int32_t l_CreateVBox(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();

    if (!lua_istable(l, 1))
    {
        luaL_error(l, "Expected a table as the first argument");
    }

    auto instance = reg.create();
    reg.emplace<UITransform>(instance);

    VBox& box = reg.emplace<VBox>(instance);

    ParseVec4(l, 1, "offset", box.offset);

    lua_getfield(l, 1, "spacing");
    box.spacing = static_cast<float>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, 1, "childStart");
    box.childStart = static_cast<ChildStart>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, 1, "controlHSize");
    box.controlHSize = lua_toboolean(l, -1);
    lua_pop(l, 1);

    lua_getfield(l, 1, "controlVSize");
    box.controlVSize = lua_toboolean(l, -1);
    lua_pop(l, 1);

    auto entityId = static_cast<lua_Integer>(instance);
    lua_pushinteger(l, entityId);

    return 1;
}

void LuaSystem::Init(Registry* registry, CommandBuffer& cmd)
{
    static auto temp = InputManager::Get().onHotReload.subscribe(CallMe::fromMethod<&LuaSystem::HotReload>(this));

    registry->AddResource<LuaScriptData>();
    LuaState& luaState = registry->AddResource<LuaState>();

    auto& unsync = luaState.unsyncState;

    unsync = luaL_newstate();
    luaL_openlibs(unsync);
    // Prevent scripts from messing with the OS directly
    lua_getglobal(unsync, "os");
    lua_pushnil(unsync);
    // Delete the os.execute so we cant run terminal commands
    lua_setfield(unsync, -2, "execute");
    lua_pushnil(unsync);
    // Delete os.exit so we can't close the game from Lua
    lua_setfield(unsync, -2, "exit");
    lua_pop(unsync, 1);

    // Delete package.loadlib to prevent loading of libs or dlls
    lua_getglobal(unsync, "package");
    lua_pushnil(unsync);
    lua_setfield(unsync, -2, "loadlib");
    lua_pop(unsync, 1);

    // Creating the UI proxy table
    lua_newtable(unsync);

    // Creating the FileSystem sub table
    lua_newtable(unsync);

    lua_pushcfunction(unsync, l_ExecuteUIScript);
    lua_setfield(unsync, -2, "ExecuteScript");

    lua_setfield(unsync, -2, "FileSystem");

    lua_pushcfunction(unsync, l_CreateWidget);
    lua_setfield(unsync, -2, "CreateWidget");

    lua_pushcfunction(unsync, l_SetParent);
    lua_setfield(unsync, -2, "SetParent");

    lua_pushcfunction(unsync, l_SetLayout);
    lua_setfield(unsync, -2, "SetLayout");

    lua_pushcfunction(unsync, l_SetVisibility);
    lua_setfield(unsync, -2, "SetVisibility");

    lua_pushcfunction(unsync, l_CreateImage);
    lua_setfield(unsync, -2, "CreateImage");

    lua_pushcfunction(unsync, l_AddNineSliceComponent);
    lua_setfield(unsync, -2, "AddNineSlice");

    lua_pushcfunction(unsync, l_DestroyEntity);
    lua_setfield(unsync, -2, "DestroyEntity");

    lua_pushcfunction(unsync, l_CreateText);
    lua_setfield(unsync, -2, "CreateText");

    lua_pushcfunction(unsync, l_SetTextStyle);
    lua_setfield(unsync, -2, "SetTextStyle");

    lua_pushcfunction(unsync, l_CreateHBox);
    lua_setfield(unsync, -2, "CreateHBox");

    lua_pushcfunction(unsync, l_CreateVBox);
    lua_setfield(unsync, -2, "CreateVBox");

    lua_setglobal(unsync, "UI");

    // Registering all UI enums
    lua_newtable(unsync);

    lua_pushinteger(unsync, 0);
    lua_setfield(unsync, -2, "Left");
    lua_pushinteger(unsync, 1);
    lua_setfield(unsync, -2, "Center");
    lua_pushinteger(unsync, 2);
    lua_setfield(unsync, -2, "Right");

    lua_setglobal(unsync, "TextHAlign");

    lua_newtable(unsync);

    lua_pushinteger(unsync, 0);
    lua_setfield(unsync, -2, "Top");
    lua_pushinteger(unsync, 1);
    lua_setfield(unsync, -2, "Middle");
    lua_pushinteger(unsync, 2);
    lua_setfield(unsync, -2, "Bottom");

    lua_setglobal(unsync, "TextVAlign");

    lua_newtable(unsync);

    lua_pushinteger(unsync, 0);
    lua_setfield(unsync, -2, "Visible");
    lua_pushinteger(unsync, 1);
    lua_setfield(unsync, -2, "Hidden");
    lua_pushinteger(unsync, 2);
    lua_setfield(unsync, -2, "Collapsed");

    lua_setglobal(unsync, "VisibilityMode");

    std::filesystem::path uiRouter = "LuaUI/ui_main.lua";
    std::filesystem::path uiDefinitions = "LuaUI/ui_definitions.lua";

    if (luaL_dofile(unsync, uiRouter.string().c_str()) != LUA_OK)
    {
        LOG(LUA_LOG, Error, std::format("UI Framework Boot Error: {}", lua_tostring(unsync, -1)));
        lua_pop(unsync, 1);
    }
    else
    {
        luaL_dofile(unsync, uiDefinitions.string().c_str());

        lua_getglobal(unsync, "UIManager");
        lua_getfield(unsync, -1, "Init");
        lua_pushvalue(unsync, -2);

        if (lua_pcall(unsync, 1, 0, 0) != LUA_OK)
        {
            LOG(LUA_LOG, Error, std::format("UI UIManager:Init Error: {}", lua_tostring(unsync, -1)));
            lua_pop(unsync, 1);
        }
    }
}

void LuaSystem::HotReload()
{
    Registry* reg = World::GetRegistry();
    LuaState& luaState = reg->AddResource<LuaState>();

    auto unsync = luaState.unsyncState;

    lua_getglobal(unsync, "UIManager");
    lua_getfield(unsync, -1, "Deinit");
    lua_pushvalue(unsync, -2);

    if (lua_pcall(unsync, 1, 0, 0) != LUA_OK)
    {
        LOG(LUA_LOG, Error, std::format("UI UIManager:Deinit Error: {}", lua_tostring(unsync, -1)));
        lua_pop(unsync, 1);
    }

    lua_getfield(unsync, -1, "Init");
    lua_pushvalue(unsync, -2);

    if (lua_pcall(unsync, 1, 0, 0) != LUA_OK)
    {
        LOG(LUA_LOG, Error, std::format("UI UIManager:Init Error: {}", lua_tostring(unsync, -1)));
        lua_pop(unsync, 1);
    }

    lua_pop(unsync, 1);
}
