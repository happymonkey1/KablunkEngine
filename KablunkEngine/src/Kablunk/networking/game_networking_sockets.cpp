#include "kablunkpch.h"

#include "Kablunk/networking/game_networking_sockets.h"
#include <steam/steamnetworkingsockets.h>

namespace
{
bool s_initialized = false;
}

namespace kb::network
{ // start namespace kb::network

auto init_game_networking_sockets_lib() -> void
{
    if (s_initialized)
        return;

    SteamDatagramErrMsg error_message{};
    if (!GameNetworkingSockets_Init(nullptr, error_message))
    {
        KB_CORE_ERROR("[networking]: GameNetworkingSockets initialization failed! {}", error_message);
        KB_CORE_ASSERT(false, "[networking]: Failed to initialize GameNetworkingSockets");
    }

    KB_CORE_INFO("[networking]: Initialized GameNetworkingSockets");
    s_initialized = true;
}

} // end namespace kb::network
