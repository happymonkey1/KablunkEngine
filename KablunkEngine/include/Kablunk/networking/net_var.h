#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/networking/network_server.h"

namespace kb::network
{ // start namespace kb::network

/**
 * References
 * https://forums.kleientertainment.com/forums/topic/48264-net_variable-types-and-sending-data-from-serverhost-to-clients/page/2/
 */

// platform-agnostic data type where data is synced across the network
template <typename T>
class net_var
{
public:
    net_var() noexcept = default;

    ~net_var()
    {
    }

    net_var(
        const uuid_t& p_uuid,
        T p_data,
        std::string p_name,
        const ref<network_server>& p_network_server
    )
        : m_uuid{ p_uuid }, m_data{ p_data }, m_name{ std::move(p_name) }, m_network_server{ p_network_server }
    {
    }

    // unidirectional setter
    auto set(T p_value) const noexcept -> void;
    // local-only setter
    auto set_local(T p_value) const noexcept -> void;
    // local-only getter
    auto get() const noexcept -> T;
private:
    uuid_t m_uuid{};
    T m_data{};
    std::string m_name{};
    ref<network_server> m_network_server{};
};

} // end namespace kb::network
