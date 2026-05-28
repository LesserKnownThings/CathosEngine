#pragma once

#include <string>

class Registry;

constexpr std::string LUA_WIDGET_TABLE = "UI_Widget";

class LuaUnsync
{
  public:
    static void InitUI(Registry* registry);
};