#pragma once

#include <cstdint>
#include <entt/entt.hpp>
#include <variant>

using MessageId = entt::id_type;
using namespace entt::literals;

// Game messages will only be processed during sim tick
struct GameMessage
{
    MessageId id;
    entt::entity entity;
    std::variant<std::monostate, int32_t, float, bool> payload;
};

// UI messages will be processed during normal tick
struct UIMessage
{
    MessageId id;
    entt::entity entity;
    std::variant<std::monostate, int32_t, float, bool> payload;
};

// This will run even if called from UI
namespace UIMessages
{
constexpr MessageId CloseGame = "close_game"_hs;

}

// This will not run if called directly called from UI
namespace GameMessages
{

};