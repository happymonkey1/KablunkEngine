#pragma once

#include "Kablunk/Core/CoreTypes.h"
#include "Kablunk/Core/crc_32_hash.h"

#include <msgpack.hpp>

namespace kb::network
{ // start namespace kb::network

struct authentication_type_name
{
    static constexpr const char* k_kb_sig_v1 = "kb-sig-v1";
};

enum class authentication_type : u32
{
    kb_sig_v1 = COMPILE_TIME_CRC32_STR(authentication_type_name::k_kb_sig_v1)
};

using underlying_auth_type_t = std::underlying_type_t<authentication_type>;

struct authentication_check_data
{
    u16 m_packet_type = 0;
    underlying_auth_type_t m_auth_version = static_cast<underlying_auth_type_t>(authentication_type::kb_sig_v1);
    std::size_t m_auth_hash = 0;

    MSGPACK_DEFINE(m_packet_type, m_auth_version, m_auth_hash);
};

struct authentication_response_data
{
    u16 m_packet_type = 0;
    std::string m_service_version{};
    client_id_t m_client_id{};

    MSGPACK_DEFINE(m_packet_type, m_service_version, m_client_id);
};

namespace details
{ // start namespace ::details
inline auto compute_kb_sig_v1_hash(
    std::string_view p_service_name
) noexcept -> std::size_t
{
    KB_CORE_INFO("[authentication]: Computing authentication hash for {}", authentication_type_name::k_kb_sig_v1);
    // shitty hash but whatever
    return std::hash<std::string>{}(
        fmt::format("{}{}",
            authentication_type_name::k_kb_sig_v1,
            p_service_name
        )
    );
}
} // end namespace ::details


inline auto compute_auth_hash(
    underlying_auth_type_t p_auth_type_hash,
    std::string_view p_service_name
) noexcept -> std::size_t
{
    switch (p_auth_type_hash)
    {
    case static_cast<underlying_auth_type_t>(authentication_type::kb_sig_v1):
    {
        return details::compute_kb_sig_v1_hash(p_service_name);
    }
    default:
    {
        KB_CORE_WARN("[authentication]: unhandled authentication type {}", p_auth_type_hash);
        return 0ull;
    }
    }
}

} // end namespace kb::network
