#include "Message.h"
#include "fpm/fixed.hpp"
#include <cstdint>
#include <cstring>

// Helper to write Little_Endian on all architectures
inline void WriteU32LE(uint8_t* buffer, uint32_t val)
{
    buffer[0] = static_cast<uint8_t>(val);
    buffer[1] = static_cast<uint8_t>(val >> 8);
    buffer[2] = static_cast<uint8_t>(val >> 16);
    buffer[3] = static_cast<uint8_t>(val >> 24);
}
inline void WriteI32LE(uint8_t* buffer, int32_t val)
{
    uint32_t uval = static_cast<uint32_t>(val);
    WriteU32LE(buffer, uval);
}

inline void WriteU16E(uint8_t* buffer, uint16_t val)
{
    uint32_t uval = static_cast<uint32_t>(val);
    WriteU32LE(buffer, uval);
}

inline void PackString(uint8_t* buffer, int32_t& offset, const std::string& str)
{
    uint16_t len = static_cast<uint16_t>(str.length());
    WriteU16E(buffer + offset, len);
    offset += 2;

    memcpy(buffer + offset, str.data(), len);
    offset += len;
}

inline uint32_t ReadU32LE(const uint8_t* buffer)
{
    uint32_t uVal = 0;

    uVal |= (static_cast<uint32_t>(buffer[0])) << 0;
    uVal |= (static_cast<uint32_t>(buffer[1])) << 8;
    uVal |= (static_cast<uint32_t>(buffer[2])) << 16;
    uVal |= (static_cast<uint32_t>(buffer[3])) << 24;

    return uVal;
}

inline int32_t ReadI32LE(const uint8_t* buffer)
{
    return static_cast<int32_t>(ReadU32LE(buffer));
}

inline uint16_t ReadU16LE(const uint8_t* buffer)
{
    return static_cast<uint16_t>(ReadU32LE(buffer));
}

inline std::string UnpackString(const uint8_t* buffer, int32_t& offset)
{
    uint16_t len = ReadU16LE(buffer + offset);
    offset += 2;

    std::string str(reinterpret_cast<const char*>(buffer + offset), len);
    offset += len;

    return str;
}

void MessagePacker::PackMessage(const Message& message, uint8_t*& outBuffer, int32_t& outSize)
{
    static uint8_t buffer[512];
    int32_t offset = 0;

    WriteI32LE(buffer, message.tick);
    offset += 4;
    buffer[offset++] = message.player;
    buffer[offset++] = static_cast<uint8_t>(message.type);

    switch (message.type)
    {
    case MessageType::Move:
    {
        const MoveMsg& move = std::get<MoveMsg>(message.data);

        WriteU32LE(buffer + offset, move.unit);
        offset += 4;
        WriteI32LE(buffer + offset, move.X());
        offset += 4;
        WriteI32LE(buffer + offset, move.Y());
        offset += 4;
        break;
    }

    case MessageType::Attack:
    {
        const AttackMsg& attack = std::get<AttackMsg>(message.data);

        WriteU32LE(buffer + offset, attack.unit);
        offset += 4;
        WriteU32LE(buffer + offset, attack.target);
        offset += 4;
        break;
    }

    case MessageType::Build:
    {
        const BuildMsg& build = std::get<BuildMsg>(message.data);

        WriteU32LE(buffer + offset, build.worker);
        offset += 4;
        WriteI32LE(buffer + offset, build.X());
        offset += 4;
        WriteI32LE(buffer + offset, build.Y());
        offset += 4;
        buffer[offset++] = build.buildType;
        break;
    }

    case MessageType::Chat:
    {
        const ChatMsg& chat = std::get<ChatMsg>(message.data);

        WriteI32LE(buffer + offset, chat.team);
        offset += 4;
        PackString(buffer, offset, chat.message);
        break;
    }

    default:
        break;
    }

    outBuffer = buffer;
    outSize = offset;
}

Message MessagePacker::UnpackMessage(uint8_t* buffer)
{
    Message cmd{};
    int32_t offset = 0;

    cmd.tick = ReadU32LE(buffer);
    offset += 4;

    cmd.player = buffer[offset++];
    cmd.type = static_cast<MessageType>(buffer[offset++]);

    switch (cmd.type)
    {
    case MessageType::Move:
    {
        MoveMsg& move = cmd.data.emplace<MoveMsg>();

        move.unit = ReadU32LE(buffer + offset);
        offset += 4;
        move.x = fpm::fixed_16_16(ReadI32LE(buffer + offset));
        offset += 4;
        move.y = fpm::fixed_16_16(ReadI32LE(buffer + offset));
        offset += 4;

        break;
    }

    case MessageType::Attack:
    {
        AttackMsg& attack = cmd.data.emplace<AttackMsg>();

        attack.unit = ReadU32LE(buffer + offset);
        offset += 4;
        attack.target = ReadU32LE(buffer + offset);
        offset += 4;
        break;
    }

    case MessageType::Build:
    {
        BuildMsg& build = cmd.data.emplace<BuildMsg>();

        build.worker = ReadU32LE(buffer + offset);
        offset += 4;
        build.x = fpm::fixed_16_16(ReadI32LE(buffer + offset));
        offset += 4;
        build.y = fpm::fixed_16_16(ReadI32LE(buffer + offset));
        offset += 4;
        build.buildType = buffer[offset++];
        break;
    }

    case MessageType::Chat:
    {
        ChatMsg& chat = cmd.data.emplace<ChatMsg>();

        chat.team = ReadI32LE(buffer + offset);
        offset += 4;
        chat.message = UnpackString(buffer, offset);

        break;
    }

    default:
        break;
    }

    return cmd;
}
