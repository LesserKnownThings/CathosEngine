#pragma once

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <string>

#define LOG(filter, level, text) \
    DebugSystem::Log(filter, level, text)

enum ELogLevel : uint8_t
{
    Log,
    Warning,
    Error,
};

class DebugSystem
{
  public:
    static void Log(const std::string& filter, ELogLevel level, const std::string& text);
};