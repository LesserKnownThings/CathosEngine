#pragma once

#include "Callme/CallMe.h"
#include "Netcode/ConnectionStatus.h"
#include "Netcode/MessageReliability.h"
#include <cstdint>
#include <steam/steamnetworkingtypes.h>

class ISteamNetworkingSockets;

struct SteamNetworkingIPAddr;
struct SteamNetConnectionStatusChangedCallback_t;

class INetworkTransport
{
  public:
    virtual ~INetworkTransport() = default;
    virtual bool Start(const SteamNetworkingIPAddr& addr) = 0;
    virtual void StartListen(HSteamNetConnection conn) = 0;
    virtual void Run();
    virtual void Shutdown() = 0;

    virtual void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) {}

    virtual void SendMessage(HSteamNetConnection conn, void* buffer, int32_t bufferSize, MessageReliability reliability = Reliable) = 0;
    virtual void SendMessage(void* buffer, int32_t bufferSize, MessageReliability reliability = Reliable) {}

    CallMe::Delegate<void(ConnectionStatus)> onConnectionStatusChanged;

  protected:
    virtual void PollConnectionStateChanges();
    virtual void PollIncomingMessages() = 0;

    ISteamNetworkingSockets* interface = nullptr;
};