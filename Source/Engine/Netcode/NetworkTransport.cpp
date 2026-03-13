#include "NetworkTransport.h"

#include <chrono>
#include <cstdint>
#include <steam/isteamnetworkingsockets.h>
#include <steam/steamnetworkingtypes.h>

void INetworkTransport::Run()
{
    static auto lastTick = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();

    // Calculate elapsed time in milliseconds
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTick).count();

    constexpr uint32_t CALLBACK_DELAY = 2000;

    if (elapsed >= CALLBACK_DELAY)
    {
        PollConnectionStateChanges();
        PollIncomingMessages();

        lastTick = currentTime;
    }
}

void INetworkTransport::PollConnectionStateChanges()
{
    interface->RunCallbacks();
}
