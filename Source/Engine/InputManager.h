#pragma once

#include "Callme/CallMe.Event.h"
#include "Callme/CallMe.h"
#include "Netcode/Message.h"
#include <cstdint>
#include <glm/ext/vector_float2.hpp>
#include <vector>

constexpr uint16_t MOUSE_JUST_PRESSED_STATE = 1 << 0;
constexpr uint16_t MOUSE_DOWN_STATE = 1 << 1;
constexpr uint16_t MOUSE_JUST_RELEASED_STATE = 1 << 2;

class InputManager
{
  public:
    static InputManager& Get();

    void PollInput();
    void FlushInput();

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

    bool IsMouseButtonJustPressed(uint8_t button) const;
    bool IsMouseButtonJustReleased(uint8_t button) const;
    bool IsMouseButtonDown(uint8_t button) const;

  private:
    InputManager();

    void ProcessKeys();
    void ProcessButton(uint8_t button, uint32_t state);

    std::vector<Message> localCommands;

    std::vector<uint32_t> mouseState;

    glm::vec2 mouseDelta;
    glm::vec2 mousePosition;

    // WASD or screen border
    glm::vec2 movementAxis;

    std::vector<uint32_t> storedKeys;
    uint32_t cachedMouseButton = 0;
};