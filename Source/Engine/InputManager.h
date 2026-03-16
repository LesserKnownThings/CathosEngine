#pragma once

#include "Callme/CallMe.Event.h"
#include "Callme/CallMe.h"
#include "Netcode/Message.h"
#include <cstdint>
#include <glm/ext/vector_float2.hpp>
#include <vector>

class InputManager
{
  public:
    static InputManager& Get();

    void PollInput();

    CallMe::Delegate<void()> onCloseGame;
    CallMe::Event<void()> onWindowResize;
    CallMe::Event<void()> onWindowMinimized;

    CallMe::Event<void(uint32_t)> onMouseClicked;
    CallMe::Event<void(uint32_t)> onMouseReleased;
    CallMe::Event<void(const glm::vec2&)> onMouseMoved;

    void HideMouse();
    void ShowMouse();

    const glm::vec2& GetMovementAxis() const { return movementAxis; }
    const glm::vec2& GetMouseDelta() const { return mouseDelta; }
    const glm::vec2& GetMousePosition() const { return mousePosition; }

  private:
    InputManager();

    void ProcessKeys();

    std::vector<Message> localCommands;

    glm::vec2 mouseDelta;
    glm::vec2 mousePosition;

    // WASD or screen border
    glm::vec2 movementAxis;

    std::vector<uint32_t> storedKeys;
    uint32_t cachedMouseButton = 0;
};