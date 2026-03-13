#include "NetworkManager.h"
#include "Callme/CallMe.h"
#include "ClientTransport.h"
#include "Debug/DebugSystem.h"
#include "Netcode/ConnectionStatus.h"
#include "NetworkTransport.h"
#include "ServerTransport.h"
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>
#include <GameNetworkingSockets/steam/steamnetworkingsockets.h>
#include <format>
#include <steam/steamnetworkingtypes.h>

constexpr uint16_t DEFAULT_SERVER_PORT = 27020;
constexpr std::string NETCODE_LOG = "Netcode";

NetworkManager& NetworkManager::Get()
{
    static NetworkManager instance{};
    return instance;
}

bool NetworkManager::HostServer(bool isListen, int32_t port)
{
    serverTransport = new ServerTransport();

    SteamNetworkingIPAddr addrServer{};
    addrServer.Clear();

    if (port == -1)
        addrServer.m_port = DEFAULT_SERVER_PORT;

    if (!serverTransport->Start(addrServer))
        return false;

    netMode = Server;

    if (isListen)
    {
        HSteamNetConnection clientConn;
        HSteamNetConnection serverConn;

        SteamNetworkingSockets()->CreateSocketPair(&clientConn, &serverConn, false, nullptr, nullptr);

        serverTransport->StartListen(serverConn);

        clientTransport = new ClientTransport();
        clientTransport->StartListen(clientConn);
    }

    return true;
}

bool NetworkManager::Join(const std::string& addr)
{
    clientTransport = new ClientTransport();

    SteamNetworkingIPAddr addrServer{};
    addrServer.Clear();
    if (!addrServer.ParseString(addr.c_str()))
    {
        LOG(NETCODE_LOG, ELogLevel::Error, std::format("Invalid server addres {}", addr));
        return false;
    }

    if (!clientTransport->Start(addrServer))
        return false;

    netMode = Client;

    if (ClientTransport* ct = static_cast<ClientTransport*>(clientTransport))
    {
        ct->onMessageReceived = CallMe::Delegate(new auto([this](void* data)
                                                          { onMessageReceived.invoke(data); }));
        ct->onConnectionStatusChanged = CallMe::Delegate(new auto([this](ConnectionStatus status)
                                                                  { onConnectionStatusChanged.invoke(status); }));
    }

    return true;
}

bool NetworkManager::Initialize(int argc, const char* argv[])
{
    bool addLatency = false;
    int32_t ping = 0;
    int32_t packetLoss = 0;
    int32_t jitter = 0;

    for (int32_t i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "--lag"))
        {
            addLatency = true;

            ++i;
            if (i >= argc)
            {
                LOG(NETCODE_LOG, Error, "--lag needs to be followed by a values: ex --lag 100 2 10");
                return false;
            }
            ping = atoi(argv[i]);
            ++i;
            if (i >= argc)
            {
                LOG(NETCODE_LOG, Error, "--lag needs to be followed by a values: ex --lag 100 2 10");
                return false;
            }
            packetLoss = atoi(argv[i]);
            ++i;
            if (i >= argc)
            {
                LOG(NETCODE_LOG, Error, "--lag needs to be followed by a values: ex --lag 100 2 10");
                return false;
            }
            jitter = atoi(argv[i]);
            continue;
        }
    }

    if (!InitConnection())
        return false;

    // Debug
    // TODO add a DEF here and remove it if it's not debug
    if (addLatency)
    {
        ISteamNetworkingUtils* utils = SteamNetworkingUtils();
        utils->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLag_Send, ping);
        utils->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLag_Recv, ping);
        utils->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketLoss_Send, packetLoss);
        utils->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_FakePacketReorder_Send, jitter);
    }

    return true;
}

void NetworkManager::Shutdown()
{
    clientTransport->Shutdown();
    delete clientTransport;
    clientTransport = nullptr;

    serverTransport->Shutdown();
    delete serverTransport;
    serverTransport = nullptr;

    GameNetworkingSockets_Kill();
}

bool NetworkManager::InitConnection()
{
    // This should not have any steam stuff, it's only using the open source steam sockets
    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg))
    {
        LOG(NETCODE_LOG, ELogLevel::Error, std::format("GameNetworkingSockets_Init failed. {}", errMsg));
        return false;
    }

    return true;
}

void NetworkManager::Run()
{
    if (serverTransport)
        serverTransport->Run();

    if (clientTransport)
        clientTransport->Run();
}

void NetworkManager::SendMessage(uint8_t* buffer, int32_t bufferSize, MessageReliability reliability)
{
    clientTransport->SendMessage(buffer, bufferSize, reliability);
}
