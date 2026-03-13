#include "ServerTransport.h"

#include "Connection.h"
#include "Debug/DebugSystem.h"
#include "NetCallbacks.h"
#include "Netcode/Message.h"
#include <GameNetworkingSockets/steam/isteamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/steamnetworkingtypes.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <format>
#include <string>

constexpr std::string SERVER_LOG = "NetServer";

bool ServerTransport::Start(const SteamNetworkingIPAddr& addr)
{
    interface = SteamNetworkingSockets();

    SteamNetworkingConfigValue_t opt[2];
    opt[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)NetCallbacks::OnConnectionStatusChanged);
    opt[1].SetInt64(k_ESteamNetworkingConfig_ConnectionUserData, reinterpret_cast<int64_t>(this));

    SteamNetworkingIPAddr localAddr{};
    localAddr.Clear();
    localAddr.m_port = addr.m_port;

    listenSocket = interface->CreateListenSocketIP(localAddr, 2, opt);

    if (listenSocket == k_HSteamListenSocket_Invalid)
    {
        LOG(SERVER_LOG, ELogLevel::Error, std::format("Failed to listen on port {}", addr.m_port));
        return false;
    }
    pollGroup = interface->CreatePollGroup();
    if (pollGroup == k_HSteamNetPollGroup_Invalid)
    {
        LOG(SERVER_LOG, ELogLevel::Error, std::format("Failed to listen on port {}", addr.m_port));
        return false;
    }

    LOG(SERVER_LOG, ELogLevel::Log, std::format("Server listening on port {}", addr.m_port));

    return true;
}

void ServerTransport::StartListen(HSteamNetConnection conn)
{
    connections.emplace(conn, Connection{
                                  "Host" });
}

void ServerTransport::Shutdown()
{
    for (auto it : connections)
    {
        interface->CloseConnection(it.first, 0, "Server Shutdown", true);
    }

    connections.clear();

    interface->CloseListenSocket(listenSocket);
    listenSocket = k_HSteamListenSocket_Invalid;

    interface->DestroyPollGroup(pollGroup);
    pollGroup = k_HSteamNetPollGroup_Invalid;
}

void ServerTransport::PollIncomingMessages()
{
    ISteamNetworkingMessage* message = nullptr;
    int32_t nMessages = interface->ReceiveMessagesOnPollGroup(pollGroup, &message, 1);

    if (nMessages == 0)
        return;

    if (nMessages < 0)
    {
        LOG(SERVER_LOG, ELogLevel::Error, "Error checking for message!");
        return;
    }

    assert(nMessages == 1 && message);

    Message msg = MessagePacker::UnpackMessage(static_cast<uint8_t*>(message->m_pData));
    if (msg.type == MessageType::Chat)
    {
        ChatMsg& chat = msg.GetChat();
        if (chat.conn != 0)
        {
            SendMessage(chat.conn, message->m_pData, message->m_cbSize);
            message->Release();
            return;
        }
    }

    // TODO add teams for connections. Should the teams be in the connection??
    // TODO also need to check if the message is intended for everyone or for certain connections.
    SendMessageToOthers(message->m_conn, message->m_pData, message->m_cbSize);

    message->Release();
}

void ServerTransport::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
{
    switch (info->m_info.m_eState)
    {
    case k_ESteamNetworkingConnectionState_None:

        break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
        {
            Message msg{};
            msg.type = MessageType::Chat;

            ChatMsg& message = msg.CreateChat();
            message.team = -1;

            auto itClient = connections.find(info->m_hConn);
            // Server should always have connection if the connection can send messages
            assert(itClient != connections.end());

            std::string debugLogAction = "";

            if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
            {
                debugLogAction = "problem detected";
                message.message = std::format("User {} has disconnected. ({})", itClient->second.nickName, info->m_info.m_szEndDebug);

                LOG(SERVER_LOG, ELogLevel::Warning, message.message);
            }
            else
            {
                debugLogAction = "closed by peer";
                message.message = std::format("User {} has disconnected.", itClient->second.nickName);

                LOG(SERVER_LOG, ELogLevel::Log, message.message);
            }

            LOG(SERVER_LOG, ELogLevel::Log, std::format("Connection {} {}, reason {}", info->m_info.m_szConnectionDescription, debugLogAction, info->m_info.m_eEndReason, info->m_info.m_szEndDebug));

            connections.erase(itClient);

            int32_t size;
            uint8_t* buffer;

            MessagePacker::PackMessage(msg, buffer, size);
            SendMessageToAllConnections(buffer, size);
        }
        else
        {
            assert(info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
        }

        interface->CloseConnection(info->m_hConn, 0, nullptr, false);
        break;

    case k_ESteamNetworkingConnectionState_Connecting:
        assert(connections.find(info->m_hConn) == connections.end());
        LOG(SERVER_LOG, ELogLevel::Log, std::format("Connection request from {}", info->m_info.m_szConnectionDescription));

        if (interface->AcceptConnection(info->m_hConn) != k_EResultOK)
        {
            interface->CloseConnection(info->m_hConn, 0, nullptr, false);
            LOG(SERVER_LOG, ELogLevel::Warning, "Can't accept connection, maybe it was already closed?");
            break;
        }

        if (!interface->SetConnectionPollGroup(info->m_hConn, pollGroup))
        {
            interface->CloseConnection(info->m_hConn, 0, nullptr, false);
            LOG(SERVER_LOG, ELogLevel::Warning, "Failed to set poll group.");
            break;
        }

        connections.emplace(info->m_hConn, Connection{ std::format("User{}", connectionTrackCounter++) });
        break;

    case k_ESteamNetworkingConnectionState_Connected:
        LOG(SERVER_LOG, ELogLevel::Log, "Client connected!");
        break;

    case k_ESteamNetworkingConnectionState_Dead:
        LOG(SERVER_LOG, ELogLevel::Warning, "Connection lost!");
        break;

    default:
        break;
    }
}

void ServerTransport::SendMessage(HSteamNetConnection conn, void* buffer, int32_t bufferSize, MessageReliability reliability)
{
    interface->SendMessageToConnection(conn, buffer, bufferSize, reliability, nullptr);
}

void ServerTransport::SendMessageToAllConnections(void* buffer, int32_t bufferSize)
{
    for (auto& c : connections)
    {
        SendMessage(c.first, buffer, bufferSize);
    }
}

void ServerTransport::SendMessageToOthers(HSteamNetConnection conn, void* buffer, int32_t bufferSize)
{
    for (auto& c : connections)
    {
        if (c.first == conn)
            continue;

        SendMessage(c.first, buffer, bufferSize);
    }
}
