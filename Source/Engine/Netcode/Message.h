#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fpm/fixed.hpp>
#include <string>
#include <variant>

enum class MessageType : uint8_t
{
    None = 0,
    Move = 1,
    Attack = 2,
    Build = 3,
    Chat = 4,
};

struct MoveMsg
{
    uint32_t unit;
    fpm::fixed_16_16 x;
    fpm::fixed_16_16 y;

    int32_t X() const
    {
        return x.raw_value();
    }

    int32_t Y() const
    {
        return y.raw_value();
    }
};

struct AttackMsg
{
    uint32_t unit;
    uint32_t target;
};

struct BuildMsg
{
    uint32_t worker;
    fpm::fixed_16_16 x;
    fpm::fixed_16_16 y;
    uint8_t buildType;

    int32_t X() const
    {
        return x.raw_value();
    }

    int32_t Y() const
    {
        return y.raw_value();
    }
};

struct ChatMsg
{
    // Negative means all
    int32_t team;
    // A specific player, can be 0 if it's meant for all
    uint32_t conn;
    std::string message;
};

struct Message
{
    uint32_t tick;
    uint8_t player;
    MessageType type;

    MoveMsg& CreateMove()
    {
        return data.emplace<MoveMsg>();
    }

    MoveMsg& GetMove()
    {
        return std::get<MoveMsg>(data);
    }

    AttackMsg& CreateAttack()
    {
        return data.emplace<AttackMsg>();
    }

    AttackMsg& GetAttack()
    {
        return std::get<AttackMsg>(data);
    }

    BuildMsg& CreateBuild()
    {
        return data.emplace<BuildMsg>();
    }

    BuildMsg& GetBuild()
    {
        return std::get<BuildMsg>(data);
    }

    ChatMsg& CreateChat()
    {
        return data.emplace<ChatMsg>();
    }

    ChatMsg& GetChat()
    {
        return std::get<ChatMsg>(data);
    }

    std::variant<MoveMsg, AttackMsg, BuildMsg, ChatMsg> data;
};

class MessagePacker
{
  public:
    static void PackMessage(const Message& message, uint8_t*& outBuffer, int32_t& outSize);
    static Message UnpackMessage(uint8_t* data);
};