#pragma once

#include "Kablunk/Core/Buffer.h"
#include "Kablunk/Core/concepts.hpp"
#include "Kablunk/networking/networking_types.h"

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

namespace kb::network
{ // start namespace kb::network

class network_client
{
public:
    enum class connection_status_t
    {
        disconnected,
        connected,
        connecting,
        failed_to_connect
    };

    /* callback function definitions */
    using data_received_callback_func_t = void (*)(const Buffer&);
    using client_connected_callback_func_t = void (*)();
    using client_disconnected_callback_func_t = void (*)();

    inline static constexpr std::size_t k_network_thread_sleep_ms = 10ull;
public:
    network_client() noexcept = default;
    ~network_client() noexcept;

    network_client(const network_client&) noexcept = delete;
    network_client(network_client&&) noexcept = default;

    /* client management */
    auto connect_to_server(const std::string& p_server_address) noexcept -> void;
    auto disconnect() noexcept -> void;
    auto is_running() const noexcept -> bool { return m_running; }
    auto get_connection_status() const noexcept -> connection_status_t { return m_connection_status; }

    /* data management */
    auto send_buffer(Buffer p_buffer, bool p_reliable = true) noexcept -> void;

    template <concepts::TrivialT T>
    auto send_data(const T& p_data, bool p_reliable = true) noexcept -> void { send_buffer(Buffer{ &p_data, sizeof(T) }, p_reliable); }

    /* factory create function */
    static auto create() noexcept -> std::unique_ptr<network_client>;

    /* overloaded operators */
    auto operator=(const network_client&) noexcept -> network_client& = delete;
    auto operator=(network_client&&) noexcept -> network_client& = default;

private:
    static auto connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* p_info) noexcept -> void;
    auto on_connection_status_changes(SteamNetConnectionStatusChangedCallback_t* p_info) noexcept -> void;

    auto network_loop() noexcept -> void;
    auto poll_incoming_messages() noexcept -> void;
    auto poll_connection_state_changes() noexcept -> void;

    auto on_fatal_error(const std::string& p_message) noexcept -> void;

private:
    std::thread m_network_thread{};
    bool m_running = false;
    connection_status_t m_connection_status = connection_status_t::disconnected;
    std::string m_server_address{};

    /* callbacks */
    data_received_callback_func_t m_data_received_callback_func = nullptr;
    client_connected_callback_func_t m_client_connected_callback_func = nullptr;
    client_disconnected_callback_func_t m_client_disconnected_callback_func = nullptr;

    std::string m_connection_debug_message{};

    ISteamNetworkingSockets* m_interface = nullptr;
    client_id_t m_connection = 0;
};

} // end namespace kb::network
