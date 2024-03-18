#pragma once

#include <Kablunk/Core/CoreTypes.h>

namespace kb::network
{ // start namespace kb::network

// enum def for engine defined packets
// DO NOT OVERRIDE VALUES
enum class packet_type : u16
{
    none = 0x0000,
    // check that we are connecting to the correct service
    kb_auth_check = 0x0001,
    // response for a service check
    kb_auth_response = 0x0002,
    // dispatch an rpc on the server
    kb_rpc_call = 0x0003,
    // dispatched rpc's response to a client
    kb_rpc_response = 0x0004,
    // ping check to clients
    kb_ping_check = 0x0005,
    // ping response from clients
    kb_ping_response = 0x0006,

    // reserve 0-255 for internal use
    kb_reserved = 0x00FF,
};

using underlying_packet_type_t = std::underlying_type_t<packet_type>;

} // end namespace kb::network
