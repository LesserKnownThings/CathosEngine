#pragma once

#include "Callme/CallMe.Event.h"
#include "Systems/ISystem.h"
#include <optional>

class CommandBuffer;
class Registry;

class UISystem : public ISystem
{
  private:
    void Init(Registry* registry, CommandBuffer& cmd) override;
    void Run(Registry* registry, CommandBuffer& cmd) override;

    void HandleWindowResized(float width, float height);

    std::optional<CallMe::Subscription> windowResizedHandle;
    bool windowResized = false;

    float cachedWidth;
    float cachedHeight;
};