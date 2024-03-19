#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/concepts.hpp"
#include "Kablunk/networking/client_info.h"


#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <thread>

#include "Kablunk/networking/packet_handler_dispatcher.h"
#include "Kablunk/networking/rpc_dispatcher.h"

namespace kb::network
{ // start namespace kb::network

class network_server : public RefCounted
{
public:
    using data_received_callback_func_t = void (*)(const client_info&, const msgpack::object&);
    using client_connected_callback_func_t = void (*)(const client_info&);
    using client_disconnected_callback_func_t = void (*)(const client_info&);

    using packet_handler_func_t = server_packet_handler_dispatcher::packet_handler_func_t;

    struct callback_info
    {
        data_received_callback_func_t m_data_received_callback_func = nullptr;
        client_connected_callback_func_t m_client_connected_callback_func = nullptr;
        client_disconnected_callback_func_t m_client_disconnected_callback = nullptr;
    };

    inline static constexpr std::size_t k_network_thread_sleep_ms = 10ull;
    inline static constexpr i32 k_default_port = 21492;

public:
    network_server() noexcept = default;
    ~network_server() noexcept override;

    network_server(const network_server&) = delete;
    network_server(network_server&&) = default;

    /* server management */

    auto start() noexcept -> void;
    auto stop() noexcept -> void;
    auto kick_client(client_id_t p_client_id) noexcept -> void;
    auto get_connected_clients() const noexcept -> const unordered_flat_map<client_id_t, client_info>& { return m_connected_clients; }
    auto get_connected_clients_count() const noexcept -> std::size_t { return m_connected_clients.size(); }
    auto is_running() const noexcept -> bool { return m_running; }

    // bind a name to a rpc function
    auto bind_rpc(const std::string& p_name, auto&& p_rpc_func) -> void
    {
        m_rpc_dispatcher.bind(p_name, std::forward<decltype(p_rpc_func)>(p_rpc_func));
    }

    // bind a packet type to an on packet received handler
    auto bind(
        underlying_packet_type_t p_packet_type,
        packet_handler_func_t p_handler
    ) noexcept -> void
    {
        m_packet_handler_dispatcher.bind(p_packet_type, p_handler);
    }

    /* data management */
    auto send_packed_buffer_to_client(
        client_id_t p_client_id,
        const msgpack::sbuffer& p_buffer,
        bool p_reliable = true
    ) const noexcept -> void
    {
        send(p_client_id, p_buffer.data(), p_buffer.size(), p_reliable);
    }

    auto send_packed_buffer_to_client(
        client_id_t p_client_id,
        const ref<msgpack::sbuffer>& p_buffer_ref,
        bool p_reliable = true
    ) const noexcept -> void
    {
        KB_CORE_ASSERT(
            p_buffer_ref,
            "[network::network_server]: Trying to send buffer to client {} but buffer is null?",
            p_client_id
        );
        send(p_client_id, p_buffer_ref->data(), p_buffer_ref->size(), p_reliable);
    }

    auto send_packed_buffer_to_all_clients(
        msgpack::sbuffer p_buffer,
        client_id_t p_exclude_client = 0u,
        bool p_reliable = true
    ) const noexcept -> void;

    auto send_packed_buffer_to_all_clients(
        const ref<msgpack::sbuffer>& p_buffer_ref,
        client_id_t p_exclude_client = 0u,
        bool p_reliable = true
    ) const noexcept -> void;

    // factory function
    static auto create(
        i32 p_port,
        std::string p_service_name,
        // struct that holds server callbacks
        callback_info&& p_callbacks
    ) noexcept -> ref<network_server>;

    /* operator overloads */
    auto operator=(const network_server&) noexcept -> network_server& = delete;
    auto operator=(network_server&&) noexcept -> network_server& = default;

private:
    network_server(
        i32 port,
        std::string&& p_service_name,
        const callback_info& p_callbacks
    ) noexcept;

    auto network_loop() noexcept -> void;
    auto poll_incoming_messages() noexcept -> void;
    auto set_client_description(client_id_t p_connection, const std::string& p_description) const noexcept -> void;
    auto poll_connection_state_changes() -> void;

    static auto connection_status_changed_callback(const SteamNetConnectionStatusChangedCallback_t* p_info) noexcept -> void;
    auto on_connection_status_change(const SteamNetConnectionStatusChangedCallback_t* p_status) noexcept -> void;

    static auto on_fatal_error(const std::string& p_message) -> void;

    auto send(
        client_id_t p_client_id,
        const void* p_data,
        size_t p_size,
        bool p_reliable = true
    ) const noexcept -> void;

    // internal handler run before user provided callback
    auto on_data_received(client_info& p_client_info, const msgpack::sbuffer& p_data_buffer) noexcept -> void;
    // dispatch different handler depending on packet type
    auto dispatch_handler_by_packet_type(
        underlying_packet_type_t p_packet_type,
        client_info& p_client_info,
        const msgpack::object& p_data_object
    ) noexcept -> void;

    // handle authentication, allowing or kicking client
    auto handle_client_authentication(
        client_info& p_client_info,
        const msgpack::object& p_auth_data_object
    ) noexcept -> void;

    // auth check that only KablunkEngine clients are trying to connect
    auto check_client_auth_packet(
        const client_info& p_client_info,
        const msgpack::object& p_data_object
    ) const noexcept -> bool;

    auto disconnect_client(
        client_id_t p_client_id
    ) noexcept -> void;

    // send successful authentication response to client
    auto send_authentication_response_to_client(
        const client_info& p_client_info
    ) const noexcept -> void;
private:
    std::thread m_network_thread;
    i32 m_port = k_default_port;
    std::string m_service_name{};

    /* callbacks */
    data_received_callback_func_t m_data_received_callback_func = nullptr;
    client_connected_callback_func_t m_client_connected_callback_func = nullptr;
    client_connected_callback_func_t m_client_disconnected_callback_func = nullptr;

    bool m_running = false;
    unordered_flat_map<client_id_t, client_info> m_connected_clients;

    ISteamNetworkingSockets* m_interface = nullptr;
    HSteamListenSocket m_listen_socket = 0u;
    HSteamNetPollGroup m_poll_group = 0u;

    // dispatcher for rpc calls
    rpc_dispatcher m_rpc_dispatcher{};
    // dispatcher for packet callbacks
    server_packet_handler_dispatcher m_packet_handler_dispatcher{};

    friend class ref<network_server>;
};

} // end namespace kb::network
