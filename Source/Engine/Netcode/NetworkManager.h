#pragma once

#include "Callme/CallMe.h"
#include "Netcode/ConnectionStatus.h"
#include "Netcode/MessageReliability.h"
#include <cstdint>
#include <string>

class INetworkTransport;

enum NetMode : uint8_t
{
    Client,
    Server,
    ListenServer,
};

enum class TransportType : uint8_t
{
    Client,
    Server,
};

class NetworkManager
{
  public:
    static NetworkManager& Get();

    bool Initialize(int argc, const char* argv[]);
    void Shutdown();

    bool HostServer(bool isListen = false, int32_t port = -1);
    bool Join(const std::string& addr);

    void SendMessage(uint8_t* buffer, int32_t bufferSize, MessageReliability reliability = Reliable);

    void Run();

    NetMode GetNetMode() const { return netMode; }

    // Client only
    CallMe::Delegate<void(void*)> onMessageReceived;
    CallMe::Delegate<void(ConnectionStatus)> onConnectionStatusChanged;

  private:
    NetworkManager() = default;

    void HandleClientMessageReceived(void* data);

    bool InitConnection();

    NetMode netMode;

    INetworkTransport* clientTransport = nullptr;
    INetworkTransport* serverTransport = nullptr;
};