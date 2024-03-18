#pragma once

#include <future>

#include "Kablunk/Core/owning_buffer.h"
#include "Kablunk/Core/concepts.hpp"
#include "Kablunk/networking/networking_types.h"

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

#include "Kablunk/networking/authentication.h"
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
    using data_received_callback_func_t = void (*)(const msgpack::object&);
    using client_connected_callback_func_t = void (*)();
    using client_disconnected_callback_func_t = void (*)();

    struct callback_info
    {
        data_received_callback_func_t m_data_received_callback_func = nullptr;
        client_connected_callback_func_t m_client_connected_callback_func = nullptr;
        client_disconnected_callback_func_t m_client_disconnected_callback_func = nullptr;
    };

    using packet_underlying_t = std::underlying_type_t<packet_type>;

    inline static constexpr std::size_t k_network_thread_sleep_ms = 10ull;

public:
    network_client() noexcept = default;
    ~network_client() noexcept override;

    network_client(const network_client&) noexcept = delete;
    network_client(network_client&&) noexcept = default;

    /* client management */
    auto connect_to_server(const std::string& p_server_address) noexcept -> void;

    auto connect_to_server(const std::string& p_server_ip, const u32 p_port) -> void
    {
        connect_to_server(fmt::format("{}:{}", p_server_ip, p_port));
    }

    auto disconnect() noexcept -> void;
    auto is_running() const noexcept -> bool { return m_running; }
    auto get_connection_status() const noexcept -> connection_status_t { return m_connection_status; }

    // serialize arguments and send an rpc request
    template <typename... Args>
    auto call_rpc(const std::string& p_rpc_name, Args&&... p_args) noexcept -> void;

    [[nodiscard]] auto send_packed_buffer(msgpack::sbuffer p_buffer, bool p_reliable = true) const noexcept -> bool;

    /* factory create function */
    [[nodiscard]] static auto create(
        std::string p_service_name,
        callback_info&& p_callback_info
    ) noexcept -> ref<network_client>;

    /* overloaded operators */
    auto operator=(const network_client&) noexcept -> network_client& = delete;
    auto operator=(network_client&&) noexcept -> network_client& = default;

private:
    network_client(
        std::string&& p_service_name,
        callback_info p_callback_info
    ) noexcept;

    static auto connection_status_changed_callback(SteamNetConnectionStatusChangedCallback_t* p_info) noexcept -> void;
    auto on_connection_status_changes(SteamNetConnectionStatusChangedCallback_t* p_info) noexcept -> void;

    auto network_loop() noexcept -> void;
    auto poll_incoming_messages() noexcept -> void;
    auto poll_connection_state_changes() noexcept -> void;

    auto on_fatal_error(const std::string& p_message) noexcept -> void;

    // internal handler which is run before user on_client_connected_callback
    auto on_client_connected() const noexcept -> void;

    // send authentication packet
    auto send_authentication_check(
        authentication_type p_auth_type = authentication_type::kb_sig_v1
    ) const noexcept -> void;

    // internal handler which is run before user provided `on_data_received_callback_func`
    auto on_data_received(msgpack::sbuffer p_data_buffer) noexcept -> void;
    // helper to dispatch internal handler for a specific packet type that is received
    auto dispatch_handler_by_packet_type(
        underlying_packet_type_t p_packet_type,
        const msgpack::object& p_data_object
    ) noexcept -> void;

    // internal handler for authentication response
    auto handle_auth_response(const msgpack::object& p_data_object) noexcept -> void;
private:
    std::thread m_network_thread{};
    bool m_running = false;
    connection_status_t m_connection_status = connection_status_t::disconnected;
    std::string m_server_address{};
    std::string m_service_name{};

    /* callbacks */
    data_received_callback_func_t m_data_received_callback_func = nullptr;
    client_connected_callback_func_t m_client_connected_callback_func = nullptr;
    client_disconnected_callback_func_t m_client_disconnected_callback_func = nullptr;

    std::string m_connection_debug_message{};

    ISteamNetworkingSockets* m_interface = nullptr;
    client_id_t m_connection = 0;
    // incrementing packet counter
    u32 m_packet_counter = 0;
    // hold futures for rpc calls that require a non-void response
    //kb::unordered_flat_map<u32, std::future<>>

    u32 m_client_id = 0;

    friend class ref<network_client>;
};

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
