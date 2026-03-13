#pragma once

#include <cstdint>

enum class ConnectionStatus : uint8_t
{
    None,
    Connecting,
    Connected,
    Failed,
};