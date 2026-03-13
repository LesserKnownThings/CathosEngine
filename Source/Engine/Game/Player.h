#pragma once

#include "Netcode/Message.h"
#include <cstdint>
#include <vector>

class NetworkManager;

class Player
{
  public:
    Player();

    std::vector<Message> pendingCommands;

  private:
    void OnReceiveMessage(uint8_t* data);
};