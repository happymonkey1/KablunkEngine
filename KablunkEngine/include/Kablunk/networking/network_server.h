#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/concepts.hpp"
#include "Kablunk/networking/client_info.h"


#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include <memory>
#include <thread>

#include "Kablunk/networking/authentication.h"
#include "Kablunk/networking/network_error.h"
#include "Kablunk/networking/packet_handler_dispatcher.h"
#include "Kablunk/networking/rpc_dispatcher.h"


namespace kb::network
{ // start namespace kb::network

namespace concepts
{ // start namespace ::concepts

template <typename T>
concept UserErrorT = std::is_enum_v<T> || std::integral<T>;

} // end namespace ::concepts

class network_server
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
        client_disconnected_callback_func_t m_client_disconnected_callback_func = nullptr;
    };

    inline static constexpr std::size_t k_network_thread_sleep_ms = 10ull;
    inline static constexpr i32 k_default_port = 21492;

public:
    network_server() noexcept = default;
    ~network_server() noexcept;

    network_server(const network_server&) = delete;

    // factory function
    static auto create(
        i32 p_port,
        std::string p_service_name,
        // struct that holds server callbacks
        const callback_info& p_callbacks
    ) noexcept -> std::unique_ptr<network_server>;

    /* server management */

    // start the server
    auto start() noexcept -> void;
    // stop the server
    auto stop() noexcept -> void;
    // kick a specific client connection
    auto kick_client(client_id_t p_client_id) const noexcept -> void;

    [[nodiscard]] auto get_connected_clients() const noexcept ->
        const unordered_flat_map<client_id_t, client_info>&
    {
        return m_connected_clients;
    }

    [[nodiscard]] auto get_connected_clients_count() const noexcept -> std::size_t
    {
        return m_connected_clients.size();
    }

    // check whether the network loop is running
    [[nodiscard]] auto is_running() const noexcept -> bool { return m_running; }

    // bind a name to a rpc function
    auto bind_rpc(const std::string& p_name, auto&& p_rpc_func) -> void
    {
        m_rpc_dispatcher.bind(p_name, std::forward<decltype(p_rpc_func)>(p_rpc_func));
    }

    // bind a packet type to an on packet received handler
    auto bind_handler(
        underlying_packet_type_t p_packet_type,
        packet_handler_func_t p_handler
    ) noexcept -> void
    {
        m_packet_handler_dispatcher.bind(p_packet_type, p_handler);
    }

    // send a type which has been reflected for msgpack (`MSGPACK_DEFINE`) to a specific client
    template <typename T>
    auto send_structured_data_to_client(
        client_id_t p_client_id,
        const T& p_data,
        bool p_reliable = true
    ) const noexcept -> void
    {
        // pack data into msgpack buffer
        // requires `MSGPACK_DEFINE` macro on the structured data
        const auto packed_buffer = util::as_buffer(p_data);

        send_packed_buffer_to_client(
            p_client_id,
            packed_buffer,
            p_reliable
        );
    }

    // send user provided error code response to a specific client
    template <concepts::UserErrorT ErrorT>
    auto send_error_response(
        const client_id_t p_client_id,
        const u32 p_response_id,
        const ErrorT p_error_code,
        const bool p_reliable = true
    ) const noexcept -> void
    {
        const auto error_code = static_cast<underlying_error_code_type_t>(p_error_code);

        KB_CORE_ASSERT(
            error_code < std::numeric_limits<underlying_error_code_type_t>::max(),
            "[network_server]: Provided error code {} exceeds max value {} in error response!",
            error_code,
            std::numeric_limits<underlying_error_code_type_t>::max()
        );

        // #TODO should we check that internal error codes are not used?

        send_structured_data_to_client(
            p_client_id,
            error_response_data{
                .m_packet_type = static_cast<underlying_packet_type_t>(internal_packet_type::kb_error_response),
                .m_response_id = p_response_id,
                .m_error_code = error_code
            },
            p_reliable
        );
    }

    /* operator overloads */
    auto operator=(const network_server&) noexcept -> network_server& = delete;
    auto operator=(network_server&&) noexcept -> network_server& = delete;

private:
    network_server(
        i32 port,
        std::string&& p_service_name,
        const callback_info& p_callbacks
    ) noexcept;

    network_server(network_server&& p_other) noexcept;

    auto network_loop() noexcept -> void;
    // poll for incoming client message(s)
    // runs on the network thread
    auto poll_incoming_messages() noexcept -> void;
    auto set_client_description(client_id_t p_connection, const std::string& p_description) const noexcept -> void;
    // poll for client connection changes
    // runs on the network thread
    auto poll_connection_state_changes() const -> void;

    static auto connection_status_changed_callback(const SteamNetConnectionStatusChangedCallback_t* p_info) noexcept -> void;
    // callback for connection status changes
    // runs on the network thread
    auto on_connection_status_change(const SteamNetConnectionStatusChangedCallback_t* p_status) noexcept -> void;

    // callback for underlying protocol implementation (steam GameNetworkingSockets) failures
    // runs on the network thread
    static auto on_fatal_error(const std::string& p_message) -> void;

    // send raw buffer to a client
    auto send(
        client_id_t p_client_id,
        const void* p_data,
        size_t p_size,
        bool p_reliable = true
    ) const noexcept -> void;

    // send a msgpack buffer to a specific client
    auto send_packed_buffer_to_client(
        client_id_t p_client_id,
        const msgpack::sbuffer& p_buffer,
        bool p_reliable = true
    ) const noexcept -> void
    {
        send(p_client_id, p_buffer.data(), p_buffer.size(), p_reliable);
    }

    // send a msgpack buffer to all client, with optional client to exclude
    auto send_packed_buffer_to_all_clients(
        msgpack::sbuffer p_buffer,
        client_id_t p_exclude_client = 0u,
        bool p_reliable = true
    ) const noexcept -> void;

    // send an internal error response to a client
    auto send_internal_error_response(
        const client_id_t p_client_id,
        const u32 p_response_id,
        const internal_error_code_t p_error_code,
        const bool p_reliable = true
    ) const noexcept -> void
    {
        send_structured_data_to_client(
            p_client_id,
            error_response_data{
                .m_packet_type = static_cast<underlying_packet_type_t>(internal_packet_type::kb_error_response),
                .m_response_id = p_response_id,
                .m_error_code = static_cast<underlying_error_code_type_t>(p_error_code),
            },
            p_reliable
        );
    }

    // internal handler run before user provided callback
    // runs on the network thread
    auto on_data_received(
        client_info& p_client_info,
        const msgpack::sbuffer& p_data_buffer
    ) noexcept -> void;

    // dispatch different handler depending on packet type
    // runs on the network thread
    auto dispatch_handler_by_packet_type(
        underlying_packet_type_t p_packet_type,
        client_info& p_client_info,
        const msgpack::object& p_data_object
    ) noexcept -> void;

    // handler for authentication, allowing or kicking client
    // runs on the network thread
    [[nodiscard]] auto client_authentication_handler(
        client_info& p_client_info,
        const msgpack::object& p_auth_data_object
    ) noexcept -> option<internal_error_code_t>;

    // auth check that only KablunkEngine clients are trying to connect
    // runs on the network thread
    [[nodiscard]] auto check_client_auth_packet(
        const client_info& p_client_info,
        const authentication_request_data& p_auth_data
    ) const noexcept -> bool;

    // forcefully disconnect a client
    auto disconnect_client(
        client_id_t p_client_id
    ) noexcept -> void;

    // send successful authentication response to client
    // runs on the network thread
    auto send_authentication_response_to_client(
        const client_info& p_client_info,
        u32 p_response_id
    ) const noexcept -> void;

private:
    std::thread m_network_thread;
    i32 m_port = k_default_port;
    std::string m_service_name{};

    /* callbacks */
    callback_info m_callbacks{};

    bool m_running = false;
    unordered_flat_map<client_id_t, client_info> m_connected_clients;

    ISteamNetworkingSockets* m_interface = nullptr;
    HSteamListenSocket m_listen_socket = 0u;
    HSteamNetPollGroup m_poll_group = 0u;

    // dispatcher for rpc calls
    rpc_dispatcher m_rpc_dispatcher{};
    // dispatcher for user provided packet callbacks
    server_packet_handler_dispatcher m_packet_handler_dispatcher{};
};

} // end namespace kb::network
