#include "kablunkpch.h"

#include "Kablunk/Core/Core.h"

#include "Kablunk/networking/game_networking_sockets.h"
#include <steam/steamnetworkingsockets.h>

namespace kb::network
{ // start namespace kb::network

namespace
{
bool s_initialized = false;
i32 s_instance_count = 0;
}

auto init_game_networking_sockets_lib() -> void
{
    if (s_initialized)
    {
        s_instance_count++;
        return;
    }

    SteamDatagramErrMsg error_message{};
    if (!GameNetworkingSockets_Init(nullptr, error_message))
    {
        KB_CORE_ERROR("[networking]: GameNetworkingSockets initialization failed! {}", error_message);
        KB_CORE_ASSERT(false, "[networking]: Failed to initialize GameNetworkingSockets");
    }

    KB_CORE_INFO("[networking]: Initialized GameNetworkingSockets");
    s_initialized = true;
    s_instance_count++;
}

auto kill_game_networking_sockets_lib() -> void
{
    if (--s_instance_count == 0)
    {
        KB_CORE_INFO("[game_networking_sockets]: Destroying library instance and freeing memory.");
        GameNetworkingSockets_Kill();
    }

    KB_CORE_ASSERT(
        s_instance_count >= 0,
        "[game_networking_sockets]: negative instance count?"
    );
}

} // end namespace kb::network
