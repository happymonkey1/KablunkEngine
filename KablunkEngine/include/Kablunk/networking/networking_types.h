#pragma once

#include "Kablunk/Core/CoreTypes.h"

#include <variant>
#include <steam/steamnetworkingtypes.h>

namespace kb::network
{ // start namespace kb::network

using client_id_t = HSteamNetConnection;

// wrapper for a rust style result
template <typename T, typename E>
using network_result = std::variant<T, E>;

using request_id_t = u32;
using response_id_t = u32;

} // end namespace kb::network
