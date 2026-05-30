#pragma once

#include "Components/Hierarchy.h"
#include "LUA/Message.h"
#include "LuaHelpers.h"
#include "Resources/AssetPath.h"
#include "Resources/AssetServer.h"
#include "Resources/Font.h"
#include "Resources/Texture.h"
#include "UI/Button.h"
#include "UI/LayoutBox.h"
#include "UI/NineSlice.h"
#include "UI/TextRenderer.h"
#include "UI/UIMaterial.h"
#include "UI/UITransform.h"
#include "UI/UIVisibility.h"
#include "World.h"

#include <filesystem>
#include <string>

inline int32_t l_GetUIFilesInFolderRecursive(lua_State* l)
{
    std::string relativePath = luaL_checkstring(l, 1);

    std::filesystem::path projectRoot = "LuaUI";
    std::filesystem::path targetPath = projectRoot / relativePath;

    lua_newtable(l);
    int32_t index = 1;

    if (std::filesystem::exists(targetPath) && std::filesystem::is_directory(targetPath))
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(targetPath))
        {
            if (entry.is_regular_file())
            {
                lua_pushinteger(l, index);
                lua_pushstring(l, entry.path().string().c_str());
                lua_settable(l, -3);

                index++;
            }
        }
    }

    return 1;
}

inline int32_t l_GetUIFile(lua_State* l)
{
    std::string relativePath = luaL_checkstring(l, 1);

    std::filesystem::path projectRoot = "LuaUI";
    std::filesystem::path targetPath = projectRoot / relativePath;

    lua_pushstring(l, targetPath.string().c_str());

    return 1;
}

inline int32_t l_SendUIMessage(lua_State* l)
{
    CommandBuffer& cmd = World::GetFrameStartCommandBuffer();

    UIMessage msg{};

    int32_t offset = 0;

    if (lua_isinteger(l, 1))
    {
        uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(l, 1));
        msg.entity = static_cast<entt::entity>(entityId);
        offset++;
    }

    if (!lua_istable(l, offset))
    {
        luaL_error(l, "Expected a table");
    }

    lua_getfield(l, offset, "id");
    msg.id = entt::hashed_string::value(lua_tostring(l, -1));
    lua_pop(l, 1);

    lua_getfield(l, offset, "value");
    if (lua_isnil(l, -1))
    {
        msg.payload = std::monostate{};
    }
    else if (lua_isboolean(l, -1))
    {
        msg.payload = static_cast<bool>(lua_toboolean(l, -1));
    }
    else if (lua_isnumber(l, -1))
    {
        msg.payload = static_cast<float>(lua_tonumber(l, -1));
    }
    else if (lua_isinteger(l, -1))
    {
        msg.payload = static_cast<int32_t>(lua_tointeger(l, -1));
    }

    auto instance = cmd.Create();
    cmd.AddComponent(instance, msg);

    return 0;
}

// Should probably rename this to empty container?
inline int32_t l_CreateWidget(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();

    entt::entity instance = reg.create();
    reg.emplace<UITransform>(instance);

    auto entityId = static_cast<lua_Integer>(instance);
    lua_pushinteger(l, entityId);

    return 1;
}

// Should probably rename this to destroy widget?
inline int32_t l_DestroyEntity(lua_State* l)
{
    CommandBuffer& cmd = World::GetFrameEndCommandBuffer();

    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(l, 1));
    auto entity = static_cast<entt::entity>(entityId);

    cmd.Destroy(entity);

    return 0;
}

inline int32_t l_CreateImage(lua_State* l)
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

inline int32_t l_AddNineSliceComponent(lua_State* l)
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

inline int32_t l_SetParent(lua_State* l)
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

inline int32_t l_SetLayout(lua_State* l)
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

inline int32_t l_SetVisibility(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();

    uint32_t entityId = static_cast<uint32_t>(luaL_checkinteger(l, 1));
    auto entity = static_cast<entt::entity>(entityId);

    UIVisiblity& visibility = reg.get<UIVisiblity>(entity);
    visibility.mode = static_cast<VisibilityMode>(lua_tointeger(l, 2));

    return 0;
}

inline int32_t l_CreateText(lua_State* l)
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

    lua_getfield(l, 1, "fontPath");
    const std::string fontPath = lua_tostring(l, -1);
    lua_pop(l, 1);

    lua_getfield(l, 1, "text");
    tr.text = lua_tostring(l, -1);
    lua_pop(l, 1);

    ParseVec4(l, 1, "color", tr.color);

    lua_getfield(l, 1, "fontSize");
    tr.fontSize = static_cast<float>(lua_tonumber(l, -1));
    lua_pop(l, 1);

    tr.font = as.Load<Font>(AssetPath{ fontPath });

    auto entityId = static_cast<lua_Integer>(instance);
    lua_pushinteger(l, entityId);

    return 1;
}

inline int32_t l_SetTextStyle(lua_State* l)
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

inline int32_t l_CreateHBox(lua_State* l)
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

inline int32_t l_CreateVBox(lua_State* l)
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

// This function will create a hierarchy and return the parent entity
inline int32_t l_CreateButton(lua_State* l)
{
    entt::registry& reg = World::GetRegistry()->Get();
    AssetServer& as = World::GetAssetServer();

    if (!lua_istable(l, 1))
    {
        luaL_error(l, "Expected a table as the first argument");
    }

    auto mainParent = reg.create();
    reg.emplace<UITransform>(mainParent);
    UIMaterial& material = reg.emplace<UIMaterial>(mainParent);
    Children& children = reg.emplace<Children>(mainParent);
    reg.emplace<Button>(mainParent);

    lua_getfield(l, 1, "texturePath");
    const char* optionalTexture = luaL_optstring(l, -1, nullptr);
    std::string texturePath = optionalTexture == nullptr ? "Assets/Engine/Textures/base_albedo.png" : optionalTexture;
    lua_pop(l, 1);

    UIEventStyle& eventStyle = reg.emplace<UIEventStyle>(mainParent);
    ParseVec4(l, 1, "normal", eventStyle.normal);
    ParseVec4(l, 1, "hover", eventStyle.hover);
    ParseVec4(l, 1, "press", eventStyle.press);

    material.textureHandle = as.Load<Texture2D>(AssetPath{ texturePath });
    material.color = eventStyle.normal;

    // Only create the text if the user requested it
    if (!lua_isnoneornil(l, 2))
    {
        auto text = reg.create();
        children.children.push_back(text);
        reg.emplace<ChildOf>(text, mainParent);
        reg.emplace<UITransform>(text, UITransform{
                                           .anchorMin = glm::vec2(0.0f),
                                           .anchorMax = glm::vec2(1.0f),
                                       });
        TextRenderer& tr = reg.emplace<TextRenderer>(text);

        lua_getfield(l, 2, "fontPath");
        std::string fontPath = lua_tostring(l, -1);
        lua_pop(l, 1);

        tr.font = as.Load<Font>(AssetPath{ fontPath });

        lua_getfield(l, 2, "text");
        tr.text = lua_tostring(l, -1);
        lua_pop(l, 1);

        lua_getfield(l, 2, "fontSize");
        tr.fontSize = static_cast<float>(lua_tonumber(l, -1));
        lua_pop(l, 1);

        ParseVec4(l, 2, "textColor", tr.color);

        TextStyle& ts = reg.emplace<TextStyle>(text);
        lua_getfield(l, 2, "horizontal");
        ts.horizontal = static_cast<TextHAlign>(lua_tointeger(l, -1));
        lua_pop(l, 1);

        lua_getfield(l, 2, "vertical");
        ts.vertical = static_cast<TextVAlign>(lua_tointeger(l, -1));
        lua_pop(l, 1);
    }

    auto entityId = static_cast<lua_Integer>(mainParent);
    lua_pushinteger(l, entityId);
    return 1;
}