#pragma once

#include "Registry/CommandBuffer.h"
#include <entt/entt.hpp>
#include <vector>

class Player;
class Registry;
class WindowBase;
struct MapFormat;

// TODO need to cleanup this class when closing the app
class MapWorld
{
  public:
    void Init();
    void Run();
    void Render();

    void EndFrameCommandBuffer();

  private:
    void HandleMapCreated(const MapFormat& map);

    std::vector<WindowBase*> windows;
    Registry* registry;
    CommandBuffer globalCmd;
    Player* player;
};