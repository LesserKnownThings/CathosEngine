#pragma once

#include <cstdint>
#include <steam/steamnetworkingtypes.h>

enum MessageReliability : int32_t
{
    Reliable = k_nSteamNetworkingSend_Reliable,
    UnReliable = k_nSteamNetworkingSend_Unreliable,
};