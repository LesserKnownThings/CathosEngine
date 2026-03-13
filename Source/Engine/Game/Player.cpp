#include "Player.h"
#include "Debug/DebugSystem.h"
#include "Netcode/Message.h"
#include "Netcode/NetworkManager.h"
#include <cstdint>
#include <string>

constexpr std::string PLAYER_LOG = "Player";

Player::Player()
{
    NetworkManager& netManager = NetworkManager::Get();
}

void Player::OnReceiveMessage(uint8_t* data)
{
    Message msg = MessagePacker::UnpackMessage(data);

    switch (msg.type)
    {
    // Chat is not a command so it can be processed directly
    case MessageType::Chat:
    {
        ChatMsg& chat = msg.GetChat();
        LOG(PLAYER_LOG, Log, chat.message);
    }

    default:
        pendingCommands.push_back(msg);
        break;
    }
}