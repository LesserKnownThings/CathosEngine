#pragma once

#include "Callme/CallMe.Event.h"
#include "Callme/CallMe.h"
#include "Netcode/Message.h"
#include <cstdint>
#include <vector>

struct InputSubscriber
{
    void Notify()
    {
    }
};

class InputManager
{
  public:
    static InputManager& Get();

    void PollInput(uint32_t tick);

    CallMe::Delegate<void()> onCloseGame;
    CallMe::Event<void()> onWindowResize;
    CallMe::Event<void()> onWindowMinimized;

  private:
    std::vector<Message>
        localCommands;
};