#pragma once

#include "Kablunk/networking/packet_type.h"
#include "Kablunk/meta/func_traits.h"
#include "Kablunk/meta/func_invoke.h"
#include "Kablunk/networking/client_info.h"

#include <msgpack.hpp>

#include <memory>

namespace kb::network
{ // start namespace kb::network

namespace details
{ // start namespace details
template <typename FuncT>
class packet_handler_dispatcher
{
public:
    packet_handler_dispatcher() noexcept = default;
    ~packet_handler_dispatcher() noexcept = default;

    // bind a handler to a packet type
    auto bind(underlying_packet_type_t p_packet_type, FuncT p_handler) noexcept -> void;

    // dispatch a handler based on the packet type
    template <typename... Args>
    auto dispatch(
        underlying_packet_type_t p_packet_type,
        Args&&... p_packet_data
    ) const noexcept -> void;

private:
    // map of packet type to handler
    kb::unordered_flat_map<underlying_packet_type_t, FuncT> m_handlers{};
};

template <typename FuncT>
auto packet_handler_dispatcher<FuncT>::bind(
    underlying_packet_type_t p_packet_type,
    FuncT p_handler
) noexcept -> void
{
    KB_CORE_ASSERT(
        !m_handlers.contains(p_packet_type),
        "[packet_handler_dispatcher]: Already contains handler for packet_type {}",
        p_packet_type
    );

    m_handlers.emplace(p_packet_type, p_handler);
}

template <typename FuncT>
template <typename... Args>
auto packet_handler_dispatcher<FuncT>::dispatch(
    underlying_packet_type_t p_packet_type,
    Args&&... p_args
) const noexcept -> void
{
    const auto handler_it = m_handlers.find(p_packet_type);
    if (handler_it == m_handlers.end())
        return;
    

    //KB_CORE_INFO("[packet_handler_dispatcher]: Dispatching handler for packet type '{}'", p_packet_type);
    handler_it->second(std::forward<Args>(p_args)...);
}
} // end namespace details

// specialize packet handler for network client calls
class client_packet_handler_dispatcher
{
public:
    using packet_handler_func_t = void(*)(const msgpack::object&);

public:
    client_packet_handler_dispatcher() = default;
    ~client_packet_handler_dispatcher() = default;

    auto bind(
        underlying_packet_type_t p_packet_type,
        packet_handler_func_t p_handler
    ) noexcept -> void
    {
        m_dispatcher.bind(p_packet_type, p_handler);
    }

    auto dispatch(
        underlying_packet_type_t p_packet_type,
        const msgpack::object& p_packet_data
    ) const noexcept -> void
    {
        m_dispatcher.dispatch(p_packet_type, p_packet_data);
    }

private:
    details::packet_handler_dispatcher<packet_handler_func_t> m_dispatcher{};
};

// specialize packet handler for network server calls
class server_packet_handler_dispatcher
{
public:
    using packet_handler_func_t = void(*)(client_info&, const msgpack::object&);

public:
    server_packet_handler_dispatcher() = default;
    ~server_packet_handler_dispatcher() = default;

    auto bind(
        const underlying_packet_type_t p_packet_type,
        packet_handler_func_t p_handler
    ) noexcept -> void
    {
        m_dispatcher.bind(p_packet_type, p_handler);
    }

    auto dispatch(
        const underlying_packet_type_t p_packet_type,
        client_info& p_client_info,
        const msgpack::object& p_packet_data
    ) const noexcept -> void
    {
        m_dispatcher.dispatch(p_packet_type, p_client_info, p_packet_data);
    }

private:
    details::packet_handler_dispatcher<packet_handler_func_t> m_dispatcher{};
};

} // end namespace kb::network
