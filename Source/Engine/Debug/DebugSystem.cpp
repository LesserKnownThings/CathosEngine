#include "DebugSystem.h"
#include <format>
#include <iostream>

namespace Utilities
{
std::string ConvertLogLevel(ELogLevel level)
{
    switch (level)
    {
    case ELogLevel::Error:
        return "Error";
    case ELogLevel::Warning:
        return "Warning";
    default:
        return "Log";
    }
}
} // namespace Utilities

void DebugSystem::Log(const std::string& filter, ELogLevel level, const std::string& text)
{
    std::string final = std::format("[{}] ", Utilities::ConvertLogLevel(level)) + std::format("({}): ", filter) + text;

    switch (level)
    {
    case ELogLevel::Warning:
        std::cout << "\033[32m";
        break;
    case ELogLevel::Error:
        std::cout << "\033[31m";
        break;
    default:
        std::cout << "\033[0m";
    }

    std::cout << final << std::endl;
    std::cout << "\033[0m";
}