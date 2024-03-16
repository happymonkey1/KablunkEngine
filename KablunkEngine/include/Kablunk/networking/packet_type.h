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
    rpc_call = 0x0003,
    // dispatched rpc's response to a client
    rpc_response = 0x0004,

};

} // end namespace kb::network
