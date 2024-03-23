#pragma once

#include <Kablunk/Core/CoreTypes.h>
#include <string>

namespace kb::network
{ // start namespace kb::network

struct server_address
{
    std::string m_server_ip{};
    u16 m_port{};
};

} // end namespace kb::network
