#pragma once

#include "Kablunk/Core/owning_buffer.h"
#include "Kablunk/Core/concepts.hpp"
#include "Kablunk/networking/networking_types.h"

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include "Kablunk/networking/authentication.h"
#include "Kablunk/networking/packet_handler_dispatcher.h"
#include "Kablunk/networking/rpc_dispatcher.h"

#include <optional>
#include <future>
#include <chrono>

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
        failed_to_connect,
        failed_to_authenticate
    };

    // whether a call should be blocking or non-blocking
    enum class network_blocking_t
    {
        blocking,
        non_blocking
    };

    /* callback function definitions */
    using data_received_callback_func_t = void (*)(const msgpack::object&);
    using client_connected_callback_func_t = void (*)();
    using client_disconnected_callback_func_t = void (*)();

    using packet_handler_func_t = client_packet_handler_dispatcher::packet_handler_func_t;

    struct callback_info
    {
        data_received_callback_func_t m_data_received_callback_func = nullptr;
        client_connected_callback_func_t m_client_connected_callback_func = nullptr;
        client_disconnected_callback_func_t m_client_disconnected_callback_func = nullptr;
    };

    using packet_underlying_t = std::underlying_type_t<internal_packet_type>;

    using promise_void_t = std::promise<void>;
    using future_void_t = std::future<void>;

    using rpc_success_t = rpc_dispatcher::response_success_t;
    using rpc_error_t = rpc_dispatcher::response_error_t;
    using rpc_promise_t = std::promise<network_result<rpc_success_t, rpc_error_t>>;
    using rpc_future_t = std::future<network_result<rpc_success_t, rpc_error_t>>;

    inline static constexpr std::size_t k_network_thread_sleep_ms = 10ull;
    inline static constexpr std::chrono::milliseconds k_rpc_timeout_ms = std::chrono::milliseconds{ 500 };
public:
    network_client() noexcept = default;
    ~network_client() noexcept;

    network_client(const network_client&) noexcept = delete;

    /* factory create function */
    [[nodiscard]] static auto create(
        std::string p_service_name,
        const callback_info& p_callback_info,
        std::optional<account_credentials> p_account_credentials
    ) noexcept -> std::unique_ptr<network_client>;

    /* client management */

    // connect to server by server address, with option to block until connection and authentication complete
    [[nodiscard]] auto connect_to_server(
        const std::string& p_server_address,
        network_blocking_t p_connection_blocking = network_blocking_t::blocking
    ) noexcept -> connection_status_t;

    // connect to server by server address, with option to block until connection and authentication complete
    [[nodiscard]] auto connect_to_server(
        const std::string& p_server_ip,
        const u32 p_port,
        const network_blocking_t p_connection_blocking = network_blocking_t::blocking
    ) noexcept -> connection_status_t
    {
        return connect_to_server(fmt::format("{}:{}", p_server_ip, p_port), p_connection_blocking);
    }

    // blocking call to wait until the client successfully connects to the server
    template <typename TimeResolutionT = std::chrono::milliseconds>
    [[nodiscard]] auto wait_for_connection(
        TimeResolutionT p_timeout = TimeResolutionT{ 5000 }
    ) noexcept -> connection_status_t;

    // send a blocking authentication packet, blocking until there is a response or times out
    template <typename TimeResolutionT = std::chrono::milliseconds>
    [[nodiscard]] auto wait_for_authentication_check(
        TimeResolutionT p_timeout_duration = TimeResolutionT{ 5000 }
    ) noexcept -> std::future_status;

    // disconnect from the server
    auto disconnect() noexcept -> void;
    // check whether the network thread is running
    [[nodiscard]] auto is_running() const noexcept -> bool { return m_running; }
    [[nodiscard]] auto get_connection_status() const noexcept -> connection_status_t { return m_connection_status; }
    [[nodiscard]] auto get_client_id() const noexcept -> client_id_t { return m_client_id; }

    auto set_account_credentials(account_credentials&& p_account_credentials) noexcept -> void
    {
        m_account_credentials = std::move(p_account_credentials);
    }

    // retrieve account credentials used for authentication with the `network_server`
    auto get_account_credentials() const noexcept -> const account_credentials& { return m_account_credentials; }

    // bind a packet type to a user provided handler.
    // handler is invoked upon receiving the specified packet type, after internal handlers are run.
    auto bind(
        underlying_packet_type_t p_packet_type,
        packet_handler_func_t p_handler
    ) noexcept -> void
    {
        m_packet_handler_dispatcher.bind(p_packet_type, p_handler);
    }

    // serialize arguments and send a non-blocking (fire and forget) rpc request
    // rpc calls that require a response should use an async method instead or an explicit rpc packet handler
    template <typename... Args>
    auto call_raw_rpc(const std::string& p_rpc_name, Args&&... p_args) noexcept -> void;

    // send an asynchronous rpc request
    // returns a promise which can be awaited for the rpc response
    template <typename... Args>
    [[nodiscard]] auto call_async_rpc(
        const std::string& p_rpc_name,
        Args&&... p_args
    ) noexcept -> rpc_future_t;

    // send a blocking rpc request
    template <typename... Args>
    [[nodiscard]] auto call_blocking_rpc(
        const std::string& p_rpc_name,
        Args&&... p_args
    ) noexcept -> network_result<rpc_success_t, rpc_error_t>;

    // helper function to send a network call to server
    // for msgpack reflected types (`MSGPACK_DEFINE` or integral types)
    template <typename T>
    auto send_structured_data(const T& p_data, bool p_reliable = true) noexcept -> void;

    // return the number of dispatched async or blocking rpc calls that are waiting for their promise to be resolved
    [[nodiscard]] auto get_rpc_promise_count() const noexcept -> size_t { return m_rpc_promise_map.size(); }

    /* overloaded operators */
    auto operator=(const network_client&) noexcept -> network_client& = delete;
    auto operator=(network_client&&) noexcept -> network_client& = default;

private:
    network_client(
        std::string&& p_service_name,
        const callback_info& p_callback_info,
        std::optional<account_credentials>&& p_account_credentials
    ) noexcept;

    network_client(network_client&& p_other) noexcept;

    // internal handler to dispatch callback to correct client running
    // (to support running multiple `network_clients`) from the same service instance
    static auto connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* p_info) noexcept -> void;

    // callback for connection status changes
    // runs on the network thread
    auto on_connection_status_changes(
        SteamNetConnectionStatusChangedCallback_t* p_info
    ) noexcept -> void;

    // main network loop, runs until disconnect is called or `network_client` is destroyed
    // runs on the network thread
    auto network_loop() noexcept -> void;

    // poll incoming messages on network thread
    auto poll_incoming_messages() noexcept -> void;
    // poll connection changes on network thread
    auto poll_connection_state_changes() noexcept -> void;

    // callback for steam game networking socket errors
    // runs on the network thread
    auto on_fatal_error(const std::string& p_message) noexcept -> void;

    // internal handler which is run before user on_client_connected_callback
    // runs on the network thread
    auto on_client_connected() noexcept -> void;

    // raw call to send packed data to server
    [[nodiscard]] auto send_packed_buffer(msgpack::sbuffer p_buffer, bool p_reliable = true) noexcept -> bool;

    // dispatch (fire and forget) non-blocking authentication packet
    auto send_raw_authentication_check(
        authentication_type p_auth_type = authentication_type::kb_sig_v1
    ) noexcept -> void;

    // send a non-blocking authentication packet
    auto send_async_authentication_check(
        const authentication_type p_auth_type = authentication_type::kb_sig_v1
    ) noexcept -> void
    {
        m_auth_check_promise = std::promise<void>{};
        send_raw_authentication_check(p_auth_type);
    }

    // internal handler which is run before user provided `on_data_received_callback_func`
    // runs on the network thread
    auto on_data_received(msgpack::sbuffer p_data_buffer) noexcept -> void;

    // helper to dispatch internal handler for a specific packet type that is received
    auto dispatch_handler_by_packet_type(
        underlying_packet_type_t p_packet_type,
        const msgpack::object& p_packet_data
    ) noexcept -> void;

    // internal handler for authentication response
    // runs on the network thread
    auto handle_auth_response(const msgpack::object& p_data_object) noexcept -> void;

    // internal handler for rpc response
    // runs on the network thread
    auto handle_rpc_response(const msgpack::object& p_rpc_response) noexcept -> void;

    // creates and internally stores a promise
    // returns a future for the corresponding raw network call
    // runs on the network thread
    [[nodiscard]] auto create_rpc_promise(u32 p_packet_index) noexcept -> rpc_future_t;

private:
    // separate networking thread which most calls run on
    std::thread m_network_thread{};
    bool m_running = false;
    connection_status_t m_connection_status = connection_status_t::disconnected;
    std::string m_server_address{};
    std::string m_service_name{};

    /* callbacks */
    callback_info m_callbacks{};

    std::string m_connection_debug_message{};

    ISteamNetworkingSockets* m_interface = nullptr;
    client_id_t m_connection = 0;
    // incrementing packet counter
    u32 m_packet_counter = 0;

    // --- promises ----------------------------------------------------
    // connection status to allow for blocking connect call
    std::promise<void> m_connection_promise{};
    // authentication check status for blocking
    std::promise<void> m_auth_check_promise{};
    // hold promises for async rpc calls
    unordered_flat_map<u32, rpc_promise_t> m_rpc_promise_map{};
    // -----------------------------------------------------------------

    u32 m_client_id = 0;
    account_credentials m_account_credentials{};

    client_packet_handler_dispatcher m_packet_handler_dispatcher{};
};

// --- Implementation details -------------------------------------------------------------------

template <typename TimeResolutionT>
auto network_client::wait_for_connection(TimeResolutionT p_timeout) noexcept -> connection_status_t
{
    const auto duration_timeout = std::chrono::duration_cast<TimeResolutionT>(p_timeout);
    const auto connection_status = m_connection_promise.get_future().wait_for(duration_timeout);

    switch (connection_status)
    {
    case std::future_status::timeout:
        return connection_status_t::failed_to_connect;
    case std::future_status::deferred:
        return connection_status_t::connecting;
    case std::future_status::ready:
        return connection_status_t::connected;
    }

    KB_CORE_ASSERT(false, "wait_for_connection reached unreachable code?");
    return connection_status_t::failed_to_connect;
}

template <typename TimeResolutionT>
auto network_client::wait_for_authentication_check(
    TimeResolutionT p_timeout_duration
) noexcept -> std::future_status
{
    const auto duration = std::chrono::duration_cast<TimeResolutionT>(p_timeout_duration);
    const auto status = m_auth_check_promise.get_future().wait_for(duration);
    switch (status)
    {
    case std::future_status::timeout:
    {
        log::core::warn(
            log::logger_tag_t::network_client,
            "Authentication check timed out!"
        );
        break;
    }
    default:
    {
        log::core::info(
            log::logger_tag_t::network_client,
            "Unblocking after receiving authentication response"
        );
        break;
    }
    }

    return status;
}

// serialize arguments and send an rpc request
template <typename... Args>
auto network_client::call_raw_rpc(
    const std::string& p_rpc_name,
    Args&&... p_args
) noexcept -> void
{
    KB_CORE_INFO("[network::network_client]: Calling rpc {}", p_rpc_name);

    // serialize arguments 
    auto arguments_pack = std::make_tuple(std::forward<Args>(p_args)...);
    msgpack::sbuffer args_buffer{};
    msgpack::packer args_packer{ args_buffer };
    args_packer.pack(arguments_pack);

    const auto args_obj_handle = msgpack::unpack(args_buffer.data(), args_buffer.size());

    const rpc_request request{
        .m_type = static_cast<packet_underlying_t>(internal_packet_type::kb_rpc_call),
        .m_request_id = m_packet_counter,
        .m_name = p_rpc_name,
        .m_arguments = args_obj_handle.get()
    };

    // serialize rpc request and send request over network
    // #TODO handle result
    send_packed_buffer(util::as_buffer(request));
}

template <typename ... Args>
auto network_client::call_async_rpc(
    const std::string& p_rpc_name,
    Args&&... p_args
) noexcept -> rpc_future_t
{
    auto future = create_rpc_promise(m_packet_counter);
    call_raw_rpc(p_rpc_name, std::forward<Args>(p_args)...);
    return future;
}

template <typename ... Args>
auto network_client::call_blocking_rpc(
    const std::string& p_rpc_name,
    Args&&... p_args
) noexcept -> network_result<rpc_success_t, rpc_error_t>
{
    auto future = create_rpc_promise(m_packet_counter);
    call_raw_rpc(p_rpc_name, std::forward<Args>(p_args)...);
    auto fut_status = future.wait_for(std::chrono::duration_cast<std::chrono::milliseconds>(k_rpc_timeout_ms));

    switch (fut_status)
    {
    case std::future_status::ready:
        break;
    case std::future_status::deferred: [[fallthrough]];
    case std::future_status::timeout:
        return network_result<rpc_success_t, rpc_error_t>{
            static_cast<underlying_error_code_type_t>(internal_error_code_t::kb_timeout)
        };
    }

    return future.get();
}

template <typename T>
auto network_client::send_structured_data(const T& p_data, bool p_reliable) noexcept -> void
{
    const auto data_buffer = util::as_buffer(p_data);
    // #TODO handle result
    send_packed_buffer(data_buffer, p_reliable);
}

} // end namespace kb::network
