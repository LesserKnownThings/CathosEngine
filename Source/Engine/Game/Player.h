#pragma once

#include "Netcode/Message.h"
#include <cstdint>
#include <glm/fwd.hpp>
#include <vector>

class NetworkManager;
class Registry;

class Player
{
  public:
    Player(Registry* inRegistry);

    void Run(uint32_t tick);
    void RunSim(uint32_t tick);

    std::vector<Message> pendingCommands;

  private:
    void CameraMovement();

    void OnReceiveMessage(uint8_t* data);

    void HandleMouseClick(uint32_t button);
    void HandleMouseRelease(uint32_t button);
    void HandleMouseMove(const glm::vec2& pos);

    Registry* registry;

    bool rotateCamera = false;
    bool isMouseDown = false;
};