#include "kablunkpch.h"

#include "Kablunk/networking/network_client.h"

#include "Kablunk/networking/game_networking_sockets.h"
#include "Kablunk/networking/network_utils.h"

namespace kb::network
{ // start namespace kb::network

constexpr std::size_t k_max_clients = 8ull;
static std::size_t s_registered_client_count = 0ull;
static std::array<network_client*, k_max_clients> s_registered_clients{};

namespace details
{ // start namespace ::details

static auto register_client_for_connection_callback(network_client* p_client_ptr) -> void
{
    KB_CORE_ASSERT(s_registered_client_count < k_max_clients, "[network_client]: Trying to register new client but exceeded max client count!");
    s_registered_clients[s_registered_client_count++] = p_client_ptr;
}

static auto unregister_client_for_connection_callback(network_client* p_client_ptr) -> void
{
    KB_CORE_ASSERT(s_registered_client_count > 0, "[network::network_client]: Trying to unregister a server when there are none in the registered server list?");

    std::size_t server_index = 0ull;
    for (const auto server_ptr : s_registered_clients)
    {
        if (server_ptr == p_client_ptr)
            break;

        ++server_index;
    }

    network_client* tail = s_registered_clients[s_registered_client_count - 1];
    s_registered_clients[server_index] = tail;
    s_registered_clients[s_registered_client_count - 1] = nullptr;

    s_registered_client_count -= 1;
}

} // end namespace details

network_client::~network_client() noexcept
{
    KB_CORE_TRACE("[network::network_client]: Joining network thread");
    if (m_network_thread.joinable())
        m_network_thread.join();

    details::unregister_client_for_connection_callback(this);
}

auto network_client::connect_to_server(const std::string& p_server_address) noexcept -> void
{
    if (m_running)
        return;

    init_game_networking_sockets_lib();

    if (m_network_thread.joinable())
        m_network_thread.join();

    KB_CORE_INFO("[network::network_client]: Trying to connect to {}", p_server_address);

    m_server_address = p_server_address;
    m_network_thread = std::thread([this] { this->network_loop(); });
}

auto network_client::disconnect() noexcept -> void
{
    m_running = false;

    if (m_network_thread.joinable())
        m_network_thread.join();
}

auto network_client::send_authentication_check(
    const authentication_type p_auth_type /*= authentication_type::kb_sig_v1*/
) const noexcept -> void
{
    KB_CORE_ASSERT(
        m_account_credentials.validate(),
        "[network_client]: Account credentails failed local validation check"
    )

    KB_CORE_INFO("[network_client]: Sending authentication check");
    const auto auth_type = static_cast<underlying_auth_type_t>(p_auth_type);
    auto auth_check_buffer = util::as_buffer(
        authentication_check_data{
            .m_packet_type = static_cast<underlying_packet_type_t>(packet_type::kb_auth_check),
            .m_auth_version = auth_type,
            .m_auth_hash = compute_auth_hash(auth_type, m_service_name),
            .m_account_credentials = m_account_credentials,
        }
    );

    if (!send_packed_buffer(std::move(auth_check_buffer)))
    {
        KB_CORE_ASSERT(false, "[network_client]: Failed to send authentication packet to server!");
    }
}

auto network_client::on_data_received(msgpack::sbuffer p_data_buffer) noexcept -> void
{
    const auto object_handle = msgpack::unpack(
        p_data_buffer.data(),
        p_data_buffer.size()
    );

    const auto object = object_handle.get();

    // all kb packets must be a msgpack array
    if (object.type != msgpack::type::ARRAY)
    {
        KB_CORE_WARN("[network_server::on_data_received]: Invalid serialized type {} found", static_cast<u32>(object.type));
        return;
    }

    // array must contain data
    if (object.via.array.size == 0)
    {
        KB_CORE_WARN("[ntework_server::on_data_received]: Invalid array size 0!");
        return;
    }

    // all kb packets must send their packet type id first
    const auto packet_type = object.via.array.ptr[0].as<underlying_packet_type_t>();

    // check packet type for kb internal handlers and dispatch
    // specific unhandled packets are early return, values above `kb_reserved` are dispatched to
    // user callback
    dispatch_handler_by_packet_type(packet_type, object);
}

auto network_client::dispatch_handler_by_packet_type(
    underlying_packet_type_t p_packet_type,
    const msgpack::object& p_data_object
) noexcept -> void
{
    // dispatch internal handlers
    switch (p_packet_type)
    {
    case static_cast<underlying_packet_type_t>(packet_type::none):
    {
        KB_CORE_WARN("[network_client]: Invalid packet type 0!");
        break;
    }
    case static_cast<underlying_packet_type_t>(packet_type::kb_auth_check):
    {
        KB_CORE_WARN("[network_client]: Recieved network check request from server?");
        break;
    }
    case static_cast<underlying_packet_type_t>(packet_type::kb_auth_response):
    {
        handle_auth_response(p_data_object);
        break;
    }
    case static_cast<underlying_packet_type_t>(packet_type::kb_rpc_call):
    {
        KB_CORE_WARN("[network_client]: Recieved rpc call on client?");
        break;
    }
    case static_cast<underlying_packet_type_t>(packet_type::kb_rpc_response):
    {
        KB_CORE_ASSERT(false, "not implemented!");
        break;
    }
    default:
    {
        if (p_packet_type > static_cast<underlying_packet_type_t>(packet_type::kb_reserved))
            m_data_received_callback_func(p_data_object);
        else
        {
            KB_CORE_ERROR(
                "[network_client]: Unhandled kb internal packet type {}",
                p_packet_type
            );
            break;
        }
    }
    }

    // dispatch user provided handlers
    m_packet_handler_dispatcher.dispatch(p_packet_type, p_data_object);
}

auto network_client::handle_auth_response(const msgpack::object& p_data_object) noexcept -> void
{
    const auto auth_response_res = util::convert_object<authentication_response_data>(p_data_object);
    if (!auth_response_res)
    {
        KB_CORE_WARN("[network_client]: Received authentication response but failed to unpack object!");
        return;
    }

    const auto auth_response = auth_response_res.value();
    // #TODO validate version is acceptable
    m_client_id = auth_response.m_client_id;

    KB_CORE_INFO("[network_client]: Received client id '{}' from server", m_client_id);
}

auto network_client::send_packed_buffer(msgpack::sbuffer p_buffer, bool p_reliable) const noexcept -> bool
{
    auto result = m_interface->SendMessageToConnection(
        m_connection,
        p_buffer.data(),
        static_cast<u32>(p_buffer.size()),
        p_reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable,
        nullptr
    );

    if (result != k_EResultOK)
    {
        KB_CORE_WARN(
            "[network_client]: Tried sending buffer to '{}' but result was {}",
            m_server_address,
            static_cast<u32>(result)
        );
        return false;
    }

    return true;
}

auto network_client::create(
    std::string p_service_name,
    callback_info&& p_callback_info,
    std::optional<account_credentials> p_account_credentials
) noexcept -> ref<network_client>
{
    auto client = ref<network_client>::Create(
        std::move(p_service_name),
        std::forward<callback_info>(p_callback_info),
        std::move(p_account_credentials)
    );
    details::register_client_for_connection_callback(client.get());
    return client;
}

network_client::network_client(
    std::string&& p_service_name,
    callback_info p_callback_info,
    std::optional<account_credentials>&& p_account_credentials
) noexcept
    : m_service_name{ std::move(p_service_name) },
    m_data_received_callback_func{ p_callback_info.m_data_received_callback_func },
    m_client_connected_callback_func{ p_callback_info.m_client_connected_callback_func },
    m_client_disconnected_callback_func{ p_callback_info.m_client_disconnected_callback_func },
    m_account_credentials{ p_account_credentials.has_value() ? *p_account_credentials : account_credentials{} }
{
    KB_CORE_ASSERT(m_data_received_callback_func, "m_data_received_callback_func cannot be null");
    KB_CORE_ASSERT(m_client_connected_callback_func, "m_client_connected_callback_func cannot be null");
    KB_CORE_ASSERT(m_client_disconnected_callback_func, "m_client_disconnected_callback_func cannot be null");
}

auto network_client::connection_status_changed_callback(
    SteamNetConnectionStatusChangedCallback_t* p_info) noexcept -> void
{
    for (size_t i = 0; i < s_registered_client_count; ++i)
    {
        const auto client = s_registered_clients[i];
        client->on_connection_status_changes(p_info);
    }
}

auto network_client::on_connection_status_changes(SteamNetConnectionStatusChangedCallback_t* p_info) noexcept -> void
{
    switch (p_info->m_info.m_eState)
    {
    case k_ESteamNetworkingConnectionState_None:
        // #NOTE: we will get callbacks here when we destroy connections. You can ignore these.
        break;
    case k_ESteamNetworkingConnectionState_Connecting:
    {
        m_connection_status = connection_status_t::connecting;
        break;
    }
    case k_ESteamNetworkingConnectionState_FindingRoute:
        break;
    case k_ESteamNetworkingConnectionState_Connected:
    {
        m_connection_status = connection_status_t::connected;
        on_client_connected();
        break;
    }
    case k_ESteamNetworkingConnectionState_ClosedByPeer: [[fallthrough]];
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
    {
        m_running = false;
        m_connection_status = connection_status_t::failed_to_connect;
        m_connection_debug_message = p_info->m_info.m_szEndDebug;

        if (p_info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
        {
            KB_CORE_WARN("[network::network_client]: Could not connect to remote host. Message='{}'", p_info->m_info.m_szEndDebug);
        }
        else if (p_info->m_eOldState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
        {
            KB_CORE_WARN("[network::network_client]: Lost connection to remote host. Message='{}'", p_info->m_info.m_szEndDebug);
        }
        else
        {
            KB_CORE_WARN("[network::network_client]: Disconnected from remote host. Message='{}'", p_info->m_info.m_szEndDebug);
        }

        // clean up the connection. this is important!
        // the connection is "closed" in the network sense, but
        // it has not been destroyed. we must close it on our end, too
        // to finish up.  the reason information do not matter in this case,
        // and we cannot linger because it's already closed on the other end,
        // so we just pass 0s.
        if (m_interface)
            m_interface->CloseConnection(p_info->m_hConn, 0, nullptr, false);
        m_connection = k_HSteamNetConnection_Invalid;
        m_connection_status = connection_status_t::disconnected;

        m_client_disconnected_callback_func();

        break;
    }
    case k_ESteamNetworkingConnectionState_FinWait:
        break;
    case k_ESteamNetworkingConnectionState_Linger:
        break;
    case k_ESteamNetworkingConnectionState_Dead:
        break;
    case k_ESteamNetworkingConnectionState__Force32Bit:
        break;
    default:
        break;
    }
}

auto network_client::network_loop() noexcept -> void
{
    KB_CORE_INFO("[network::network_client]: Starting network loop.");

    // reset connection status
    m_connection_status = connection_status_t::connecting;

    // use default instance
    m_interface = SteamNetworkingSockets();

    SteamNetworkingIPAddr server_address;
    if (!server_address.ParseString(m_server_address.c_str()))
    {
        on_fatal_error(fmt::format("Invalid IP address! Failed to parse '{}'", m_server_address));
        m_connection_debug_message = "Invalid IP Address";
        m_connection_status = connection_status_t::failed_to_connect;
        return;
    }

    SteamNetworkingConfigValue_t config{};
    // must correspond to the number of config values set below
    constexpr i32 k_options_count = 1;
    config.SetPtr(
        k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
        static_cast<void*>(&network_client::connection_status_changed_callback)
    );

    while (m_connection == k_HSteamNetConnection_Invalid)
    {
        KB_CORE_TRACE(
            "[network::network_client]: Attempting to connect to '{}'",
            m_server_address
        );
        m_connection = m_interface->ConnectByIPAddress(server_address, k_options_count, &config);
        if (m_connection == k_HSteamNetConnection_Invalid)
        {
            m_connection_debug_message = "Failed to create connection";
            on_fatal_error(m_connection_debug_message);
            m_connection_status = connection_status_t::failed_to_connect;

            std::this_thread::sleep_for(std::chrono::milliseconds(k_network_thread_sleep_ms * 10));
        }
    }
    
    KB_CORE_INFO("[network::network_client]: Connected to '{}'", m_server_address);
    m_running = true;
    while (m_running)
    {
        poll_incoming_messages();
        poll_connection_state_changes();
        std::this_thread::sleep_for(std::chrono::milliseconds(k_network_thread_sleep_ms));
    }

    KB_CORE_INFO("[network::network_client]: Finished blocking network thread.");

    m_interface->CloseConnection(m_connection, 0, nullptr, false);
    KB_CORE_TRACE("[network::network_client]: Closed connection to {}", m_server_address);
    m_connection_status = connection_status_t::disconnected;

    m_interface = nullptr;

    kill_game_networking_sockets_lib();
}

auto network_client::poll_incoming_messages() noexcept -> void
{
    while (m_running)
    {
        ISteamNetworkingMessage* incoming_message = nullptr;
        constexpr i32 k_max_receive_message_count = 1ul;
        const i32 message_count = m_interface->ReceiveMessagesOnConnection(m_connection, &incoming_message, k_max_receive_message_count);
        if (message_count == 0)
            break;

        if (message_count < 0)
        {
            m_running = false;
            return;
        }

        if (incoming_message->m_cbSize)
        {
            msgpack::sbuffer buffer{};
            buffer.write(
                static_cast<const char*>(incoming_message->m_pData),
                static_cast<std::size_t>(incoming_message->m_cbSize)
            );

            on_data_received(std::move(buffer));
        }

        incoming_message->Release();
    }
}

auto network_client::poll_connection_state_changes() noexcept -> void
{
    m_interface->RunCallbacks();
}

auto network_client::on_fatal_error(const std::string& p_message) noexcept -> void
{
    KB_CORE_ERROR("[network_client]: Fatal error! Message='{}'", p_message);
}

auto network_client::on_client_connected() const noexcept -> void
{
    // #TODO wait for response
    send_authentication_check();

    m_client_connected_callback_func();
}

} // end namespace kb::network
