#include "NetworkTransport.h"
#include <steam/isteamnetworkingsockets.h>

namespace NetCallbacks
{
static void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
{
    INetworkTransport* self = (INetworkTransport*)info->m_info.m_nUserData;
    if (self != nullptr)
    {
        self->OnConnectionStatusChanged(info);
    }
}
} // namespace NetCallbacks