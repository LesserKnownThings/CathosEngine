#include "ClientTransport.h"

#include "Debug/DebugSystem.h"
#include "NetCallbacks.h"
#include "Netcode/ConnectionStatus.h"
#include <cstdint>
#include <cstdio>
#include <format>
#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingtypes.h>
#include <string>

constexpr std::string CLIENT_LOG = "NetClient";

bool ClientTransport::Start(const SteamNetworkingIPAddr& addr)
{
    interface = SteamNetworkingSockets();

    SteamNetworkingConfigValue_t options[2];
    options[0].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)NetCallbacks::OnConnectionStatusChanged);
    options[1].SetInt64(k_ESteamNetworkingConfig_ConnectionUserData, reinterpret_cast<int64_t>(this));

    serverConnection = interface->ConnectByIPAddress(addr, 2, options);

    if (serverConnection == k_HSteamNetConnection_Invalid)
    {
        LOG(CLIENT_LOG, ELogLevel::Error, "Failed to create connection!");
        return false;
    }

    return true;
}

void ClientTransport::StartListen(HSteamNetConnection conn)
{
    interface = SteamNetworkingSockets();
    serverConnection = conn;
}

void ClientTransport::Shutdown()
{
}

void ClientTransport::PollIncomingMessages()
{
    ISteamNetworkingMessage* incomingMsg = nullptr;
    int numMsgs = interface->ReceiveMessagesOnConnection(serverConnection, &incomingMsg, 1);

    if (numMsgs == 0)
        return;

    onMessageReceived.invoke(incomingMsg->m_pData);
    incomingMsg->Release();
}

void ClientTransport::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
{
    switch (info->m_info.m_eState)
    {
    case k_ESteamNetworkingConnectionState_None:

        break;

    case k_ESteamNetworkingConnectionState_Connecting:
        LOG(CLIENT_LOG, ELogLevel::Log, "Client is connecting.");
        onConnectionStatusChanged.invoke(ConnectionStatus::Connecting);
        break;

    case k_ESteamNetworkingConnectionState_Connected:
        LOG(CLIENT_LOG, ELogLevel::Log, "Client connected.");
        onConnectionStatusChanged.invoke(ConnectionStatus::Connected);
        break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
        if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
        {
            LOG(CLIENT_LOG, ELogLevel::Warning, std::format("Can't reach the host. ({})", info->m_info.m_szEndDebug));
        }
        else if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
        {
            LOG(CLIENT_LOG, ELogLevel::Warning, std::format("The host did not answer. ({})", info->m_info.m_szEndDebug));
        }
        else
        {
            LOG(CLIENT_LOG, ELogLevel::Log, std::format("The host closed the connection, disconnecting. ({})", info->m_info.m_szEndDebug));
        }

        onConnectionStatusChanged.invoke(ConnectionStatus::Failed);
        break;

    default:
        break;
    }
}

void ClientTransport::SendMessage(HSteamNetConnection conn, void* buffer, int32_t bufferSize, MessageReliability reliability)
{
    interface->SendMessageToConnection(conn, buffer, bufferSize, reliability, nullptr);
}

void ClientTransport::SendMessage(void* buffer, int32_t bufferSize, MessageReliability reliability)
{
    interface->SendMessageToConnection(serverConnection, buffer, bufferSize, reliability, nullptr);
}
