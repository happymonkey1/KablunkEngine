#pragma once

#include "Kablunk/networking/networking_types.h"
#include <string>

namespace kb::network
{ // start namespace kb::network

struct client_info
{
    client_id_t m_client_id{};
    std::string m_connection_description{};
};

} // end namespace kb::network
