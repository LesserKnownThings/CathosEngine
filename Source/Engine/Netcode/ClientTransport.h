#pragma once

#include "Callme/CallMe.h"
#include "NetworkTransport.h"
#include <steam/steamnetworkingtypes.h>

class ClientTransport : public INetworkTransport
{
  public:
    ~ClientTransport() = default;

    bool Start(const SteamNetworkingIPAddr& addr) override;
    void StartListen(HSteamNetConnection conn) override;
    void Shutdown() override;

    virtual void SendMessage(HSteamNetConnection conn, void* buffer, int32_t bufferSize, MessageReliability reliability = Reliable) override;
    virtual void SendMessage(void* buffer, int32_t bufferSize, MessageReliability reliability = Reliable) override;

    void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) override;

    CallMe::Delegate<void(void* data)> onMessageReceived;

  private:
    void PollIncomingMessages() override;

    HSteamNetConnection serverConnection;
};