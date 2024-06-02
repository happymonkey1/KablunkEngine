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
        failed_to_connect
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

    using packet_underlying_t = std::underlying_type_t<packet_type>;

    using promise_t = std::promise<void>;
    using future_t = std::future<void>;

    inline static constexpr std::size_t k_network_thread_sleep_ms = 10ull;

public:
    network_client() noexcept = default;
    ~network_client() noexcept;

    network_client(const network_client&) noexcept = delete;

    /* client management */
    auto connect_to_server(const std::string& p_server_address) noexcept -> void;

    auto connect_to_server(const std::string& p_server_ip, const u32 p_port) -> void
    {
        connect_to_server(fmt::format("{}:{}", p_server_ip, p_port));
    }

    // blocking call to wait until the client successfully connects to the server
    template <typename TimeResolutionT = std::chrono::milliseconds>
    [[nodiscard]] auto wait_for_connection(
        TimeResolutionT p_timeout = TimeResolutionT{ 5000 }
    ) noexcept -> connection_status_t;

    auto disconnect() noexcept -> void;
    // check whether the network thread is running
    auto is_running() const noexcept -> bool { return m_running; }
    auto get_connection_status() const noexcept -> connection_status_t { return m_connection_status; }
    auto get_client_id() const noexcept -> client_id_t { return m_client_id; }

    auto set_account_credentials(account_credentials&& p_account_credentials) noexcept -> void
    {
        m_account_credentials = std::move(p_account_credentials);
    }

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

    // serialize arguments and send an rpc request
    template <typename... Args>
    auto call_rpc(const std::string& p_rpc_name, Args&&... p_args) noexcept -> void;

    [[nodiscard]] auto send_packed_buffer(msgpack::sbuffer p_buffer, bool p_reliable = true) const noexcept -> bool;

    /* factory create function */
    [[nodiscard]] static auto create(
        std::string p_service_name,
        callback_info&& p_callback_info,
        std::optional<account_credentials> p_account_credentials
    ) noexcept -> std::unique_ptr<network_client>;

    /* overloaded operators */
    auto operator=(const network_client&) noexcept -> network_client& = delete;
    auto operator=(network_client&&) noexcept -> network_client& = default;

private:
    network_client(
        std::string&& p_service_name,
        callback_info p_callback_info,
        std::optional<account_credentials>&& p_account_credentials
    ) noexcept;

    network_client(network_client&& p_other) noexcept;

    static auto connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* p_info) noexcept -> void;
    auto on_connection_status_changes(
        SteamNetConnectionStatusChangedCallback_t* p_info
    ) noexcept -> void;

    // main network loop, runs until shutdown is called or `network_client` is destroyed
    auto network_loop() noexcept -> void;

    // poll incoming messages on network thread
    auto poll_incoming_messages() noexcept -> void;
    // poll connection changes on network thread
    auto poll_connection_state_changes() noexcept -> void;

    auto on_fatal_error(const std::string& p_message) noexcept -> void;

    // internal handler which is run before user on_client_connected_callback
    auto on_client_connected() noexcept -> void;

    // send non-blocking authentication packet
    auto send_raw_authentication_check(
        authentication_type p_auth_type = authentication_type::kb_sig_v1
    ) const noexcept -> void;

    // send a non-blocking authentication packet
    [[nodiscard]] auto send_async_authentication_check(
        const authentication_type p_auth_type = authentication_type::kb_sig_v1
    ) noexcept -> future_t
    {
        auto fut = create_raw_network_call_future(m_packet_counter);
        send_raw_authentication_check(p_auth_type);
        return fut;
    }

    // send a blocking authentication packet, blocking until there is a response or times out
    template <typename TimeResolutionT = std::chrono::milliseconds>
    [[nodiscard]] auto send_blocking_authentication_check(
        const authentication_type p_auth_type = authentication_type::kb_sig_v1,
        TimeResolutionT p_timeout_duration = TimeResolutionT{ 5000 }
    ) noexcept -> std::future_status
    {
        const auto duration = std::chrono::duration_cast<TimeResolutionT>(p_timeout_duration);
        return send_async_authentication_check(p_auth_type).wait_for(duration);
    }

    // internal handler which is run before user provided `on_data_received_callback_func`
    auto on_data_received(msgpack::sbuffer p_data_buffer) noexcept -> void;

    // helper to dispatch internal handler for a specific packet type that is received
    auto dispatch_handler_by_packet_type(
        underlying_packet_type_t p_packet_type,
        const msgpack::object& p_data_object
    ) noexcept -> void;

    // internal handler for authentication response
    auto handle_auth_response(const msgpack::object& p_data_object) noexcept -> void;

    // creates and internally stores a promise
    // returns a future for the corresponding raw network call
    [[nodiscard]] auto create_raw_network_call_future(u32 p_packet_index) noexcept -> future_t;
private:
    std::thread m_network_thread{};
    bool m_running = false;
    connection_status_t m_connection_status = connection_status_t::disconnected;
    std::string m_server_address{};
    std::string m_service_name{};

    /* callbacks */
    // #TODO why is this not the callback struct?
    data_received_callback_func_t m_data_received_callback_func = nullptr;
    client_connected_callback_func_t m_client_connected_callback_func = nullptr;
    client_disconnected_callback_func_t m_client_disconnected_callback_func = nullptr;

    std::string m_connection_debug_message{};

    ISteamNetworkingSockets* m_interface = nullptr;
    client_id_t m_connection = 0;
    // incrementing packet counter
    u32 m_packet_counter = 0;

    // --- promises ----------------------------------------------------
    // connection status to allow for blocking connect call
    std::promise<void> m_connection_promise{};
    // hold promises for async network calls
    unordered_flat_map<u32, promise_t> m_raw_network_call_promise_map{};
    // -----------------------------------------------------------------

    u32 m_client_id = 0;
    account_credentials m_account_credentials{};

    client_packet_handler_dispatcher m_packet_handler_dispatcher{};

    friend class ref<network_client>;
};

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

// serialize arguments and send an rpc request
template <typename... Args>
auto network_client::call_rpc(
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
        .m_type = static_cast<packet_underlying_t>(packet_type::kb_rpc_call),
        .m_id = m_packet_counter,
        .m_name = p_rpc_name,
        .m_arguments = args_obj_handle.get()
    };

    // serialize rpc request
    msgpack::sbuffer rpc_buffer{};
    msgpack::packer rpc_packer{ rpc_buffer };
    rpc_packer.pack(request);
    //auto arguments = msgpack::type::make_tuple(std::forward<Args>(p_args)...);
    //rpc_packer.pack(arguments);

    // send request over network
    if (send_packed_buffer(std::move(rpc_buffer)))
        ++m_packet_counter;
}

} // end namespace kb::network
