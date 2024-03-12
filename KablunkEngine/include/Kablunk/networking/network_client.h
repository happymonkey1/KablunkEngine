#pragma once

#include "Kablunk/Core/owning_buffer.h"
#include "Kablunk/Core/concepts.hpp"
#include "Kablunk/networking/networking_types.h"

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include "Kablunk/networking/rpc_dispatcher.h"

namespace kb::network
{ // start namespace kb::network

class network_client : public RefCounted
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
    using data_received_callback_func_t = void (*)(const owning_buffer&);
    using client_connected_callback_func_t = void (*)();
    using client_disconnected_callback_func_t = void (*)();

    inline static constexpr std::size_t k_network_thread_sleep_ms = 10ull;
public:
    network_client() noexcept = default;
    ~network_client() noexcept override;

    network_client(const network_client&) noexcept = delete;
    network_client(network_client&&) noexcept = default;

    /* client management */
    auto connect_to_server(const std::string& p_server_address) noexcept -> void;
    auto disconnect() noexcept -> void;
    auto is_running() const noexcept -> bool { return m_running; }
    auto get_connection_status() const noexcept -> connection_status_t { return m_connection_status; }

    // serialize arguments and send an rpc request
    auto call_rpc(const std::string& p_rpc_name, auto&&... p_args) const noexcept -> void;

    auto send_packed_buffer(msgpack::sbuffer p_buffer, bool p_reliable = true) const noexcept -> void;

    /* factory create function */
    [[nodiscard]] static auto create() noexcept -> ref<network_client>;

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

// serialize arguments and send an rpc request
auto network_client::call_rpc(
    const std::string& p_rpc_name,
    auto&&... p_args
) const noexcept -> void
{
    KB_CORE_INFO("[network::network_client]: Calling rpc {}", p_rpc_name);

    // pack arguments
    // #TODO is there better way than serializing and immediately deserializing?
    auto arguments = msgpack::type::make_tuple(p_args...);
    msgpack::sbuffer args_buffer{};
    msgpack::pack(args_buffer, arguments);

    const msgpack::object_handle args_handle = msgpack::unpack(
        args_buffer.data(),
        args_buffer.size()
    );

    // serialize rpc request
    const auto request = rpc_request{
        .m_header = {
            .m_id = 0x01,
            .m_name = p_rpc_name,
        },
        .m_arguments = args_handle.get(),
    };

    // send request over network
    send_packed_buffer(request.as_buffer());
}

} // end namespace kb::network
