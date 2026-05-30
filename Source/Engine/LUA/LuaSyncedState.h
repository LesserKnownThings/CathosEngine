#pragma once

#include <filesystem>
#include <lua.hpp>
#include <string>

inline int32_t l_GetFilesInFolderRecursive(lua_State* l)
{
    std::string relativePath = luaL_checkstring(l, 1);

    std::filesystem::path projectRoot = "LuaGame";
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

inline int32_t l_GetFile(lua_State* l)
{
    std::string relativePath = luaL_checkstring(l, 1);

    std::filesystem::path projectRoot = "LuaGame";
    std::filesystem::path targetPath = projectRoot / relativePath;

    lua_pushstring(l, targetPath.string().c_str());

    return 1;
}
