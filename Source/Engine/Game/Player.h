#pragma once

#include "Netcode/Message.h"
#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <glm/fwd.hpp>
#include <vector>

class NetworkManager;

class Player
{
  public:
    Player();

    void Run(entt::registry& registry, uint32_t tick);
    void RunSim(entt::registry& registry, uint32_t tick);

    std::vector<Message> pendingCommands;

  private:
    void CameraMovement(entt::registry& registry);

    void OnReceiveMessage(uint8_t* data);

    void HandleMouseClick(uint32_t button);
    void HandleMouseRelease(uint32_t button);
    void HandleMouseMove(const glm::vec2& pos);

    bool rotateCamera = false;
    bool isMouseDown = false;
};