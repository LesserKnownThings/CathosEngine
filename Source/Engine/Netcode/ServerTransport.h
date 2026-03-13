#pragma once

#include "Connection.h"
#include "NetworkTransport.h"
#include <GameNetworkingSockets/steam/isteamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/steamnetworkingtypes.h>
#include <cstdint>
#include <map>

class ServerTransport : public INetworkTransport
{
  public:
    ~ServerTransport() = default;

    bool Start(const SteamNetworkingIPAddr& addr) override;
    void StartListen(HSteamNetConnection conn) override;
    void Shutdown() override;

    void SendMessage(HSteamNetConnection conn, void* buffer, int32_t bufferSize, MessageReliability reliability = Reliable) override;
    void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info) override;

  private:
    void PollIncomingMessages() override;
    void SendMessageToAllConnections(void* buffer, int32_t bufferSize);
    void SendMessageToOthers(HSteamNetConnection conn, void* buffer, int32_t bufferSize);

    HSteamListenSocket listenSocket;
    HSteamNetPollGroup pollGroup;

    std::map<HSteamNetConnection, Connection> connections;
    uint32_t connectionTrackCounter = 0;
};