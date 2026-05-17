#pragma once

#include "Registry/CommandBuffer.h"
#include <entt/entt.hpp>

class Player;
class Registry;
class WindowBase;
struct MapFormat;

// TODO need to cleanup this class when closing the app
class EditorWorld
{
  public:
    void Init();
    void Run();
    void Render();

    void EndFrameCommandBuffer();

    static bool showMapSectors;
    static bool showMapPortals;
    static bool showPathNetwork;

  private:
    void HandleMapCreated(const MapFormat& map);

    Registry* registry;
    CommandBuffer globalCmd;
    Player* player;
};