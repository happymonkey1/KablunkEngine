#pragma once

#include "Kablunk/networking/networking_types.h"

#include <msgpack.hpp>

#include <string>

namespace kb::network
{ // start namespace kb::network

struct account_credentials
{
    std::string m_username{};

    auto validate() const noexcept -> bool { return !m_username.empty(); }

    MSGPACK_DEFINE(m_username);
};

struct client_info
{
    client_id_t m_client_id{};
    std::string m_connection_description{};
    account_credentials m_account_credentials{};
    bool m_authenticated = false;
};

} // end namespace kb::network
