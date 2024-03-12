#pragma once

namespace kb::network
{ // start namespace kb::network

// wrapper for GameNetworkingSocket library initialization
auto init_game_networking_sockets_lib() -> void;
// wrapper for GameNetworkingSocket library destruction
auto kill_game_networking_sockets_lib() -> void;

} // end namespace kb::network
