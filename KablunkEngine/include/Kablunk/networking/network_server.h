#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/owning_buffer.h"
#include "Kablunk/Core/concepts.hpp"
#include "Kablunk/networking/client_info.h"


#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <thread>

namespace kb::network
{ // start namespace kb::network

class network_server : public RefCounted
{
public:
    using data_received_callback_func_t = void (*)(const client_info&, const owning_buffer&);
    using client_connected_callback_func_t = void (*)(const client_info&);
    using client_disconnected_callback_func_t = void (*)(const client_info&);

    inline static constexpr std::size_t k_network_thread_sleep_ms = 10ull;
    inline static constexpr i32 k_default_port = 21492;
public:
    network_server() noexcept = default;
    ~network_server() noexcept;

    network_server(const network_server&) = delete;
    network_server(network_server&&) = default;

    /* server management */
    auto start() noexcept -> void;
    auto stop() noexcept -> void;
    auto kick_client(client_id_t p_client_id) noexcept -> void;
    auto get_connected_clients() const noexcept -> const unordered_flat_map<client_id_t, client_info>& { return m_connected_clients; }
    auto is_running() const noexcept -> bool { return m_running; }

    /* data management */
    auto send_buffer_to_client(client_id_t p_client_id, owning_buffer p_buffer, bool p_reliable = true) noexcept -> void;
    auto send_buffer_to_all_clients(owning_buffer p_buffer, client_id_t p_exclude_client = 0u, bool p_reliable = true) noexcept -> void;

    template <concepts::TrivialT T>
    auto send_data_to_client(client_id_t p_client_id, const T& p_data, bool p_reliable = true) noexcept -> void
    {
        send_buffer_to_client(p_client_id, owning_buffer{ &p_data, sizeof(T) }, p_reliable);
    }

    template <concepts::TrivialT T>
    auto send_data_to_all_clients(const T& p_data, client_id_t p_exclude_client = 0u, bool p_reliable = true) noexcept -> void
    {
        send_buffer_to_all_clients(owning_buffer{ &p_data, sizeof(T) }, p_exclude_client, p_reliable);
    }

    // factory function
    static auto create(
        i32 p_port,
        data_received_callback_func_t p_data_received_callback_func,
        client_connected_callback_func_t p_client_connected_callback_func,
        client_disconnected_callback_func_t p_client_disconnected_callback
    ) noexcept -> std::unique_ptr<network_server>;

    /* operator overloads */
    auto operator=(const network_server&) noexcept -> network_server& = delete;
    auto operator=(network_server&&) noexcept -> network_server& = default;

private:
    network_server(
        i32 port,
        data_received_callback_func_t p_data_received_callback_func,
        client_connected_callback_func_t p_client_connected_callback_func,
        client_disconnected_callback_func_t p_client_disconnected_callback
    ) noexcept;

    auto network_loop() noexcept -> void;
    auto poll_incoming_messages() noexcept -> void;
    auto set_client_description(client_id_t p_connection, const std::string& p_description) noexcept -> void;
    auto poll_connection_state_changes() -> void;

    static auto connection_status_changed_callback(const SteamNetConnectionStatusChangedCallback_t* p_info) noexcept -> void;
    auto on_connection_status_change(const SteamNetConnectionStatusChangedCallback_t* p_status) noexcept -> void;

    auto on_fatal_error(const std::string& p_message) -> void;
private:
    std::thread m_network_thread;
    i32 m_port = k_default_port;

    /* callbacks */
    data_received_callback_func_t m_data_received_callback_func = nullptr;
    client_connected_callback_func_t m_client_connected_callback_func = nullptr;
    client_connected_callback_func_t m_client_disconnected_callback_func = nullptr;

    bool m_running = false;
    unordered_flat_map<client_id_t, client_info> m_connected_clients;

    ISteamNetworkingSockets* m_interface = nullptr;
    HSteamListenSocket m_listen_socket = 0u;
    HSteamNetPollGroup m_poll_group = 0u;
};

} // end namespace kb::network
