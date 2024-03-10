#include "kablunkpch.h"

#include "Kablunk/networking/network_client.h"

#include "Kablunk/networking/game_networking_sockets.h"

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
    KB_CORE_ASSERT(s_registered_client_count > 0, "[network_server]: Trying to unregister a server when there are none in the registered server list?");

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

    m_server_address = p_server_address;
    m_network_thread = std::thread([this] { this->network_loop(); });
}

auto network_client::disconnect() noexcept -> void
{
    m_running = false;

    if (m_network_thread.joinable())
        m_network_thread.join();
}

auto network_client::send_buffer(owning_buffer p_buffer, bool p_reliable) noexcept -> void
{
    auto result = m_interface->SendMessageToConnection(
        m_connection,
        p_buffer.get(),
        static_cast<u32>(p_buffer.size()),
        p_reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable,
        nullptr
    );
}

auto network_client::create() noexcept -> std::unique_ptr<network_client>
{
    auto client = std::make_unique<network_client>();
    details::register_client_for_connection_callback(client.get());
    return std::move(client);
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
        m_client_connected_callback_func();
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
            KB_CORE_WARN("[network_server]: Could not connect to remote host. Message='{}'", p_info->m_info.m_szEndDebug);
        }
        else if (p_info->m_eOldState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
        {
            KB_CORE_WARN("[network_server]: Lost connection to remote host. Message='{}'", p_info->m_info.m_szEndDebug);
        }
        else
        {
            KB_CORE_WARN("[network_server]: Disconnected from remote host. Message='{}'", p_info->m_info.m_szEndDebug);
        }

        // clean up the connection. this is important!
        // the connection is "closed" in the network sense, but
        // it has not been destroyed. we must close it on our end, too
        // to finish up.  the reason information do not matter in this case,
        // and we cannot linger because it's already closed on the other end,
        // so we just pass 0s.
        m_interface->CloseConnection(p_info->m_hConn, 0, nullptr, false);
        m_connection = k_HSteamNetConnection_Invalid;
        m_connection_status = connection_status_t::disconnected;

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
    config.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, static_cast<void*>(&network_client::connection_status_changed_callback));
    m_connection = m_interface->ConnectByIPAddress(server_address, k_options_count, &config);
    if (m_connection == k_HSteamNetConnection_Invalid)
    {
        m_connection_debug_message = "Failed to create connection";
        on_fatal_error(m_connection_debug_message);
        m_connection_status = connection_status_t::failed_to_connect;
        return;
    }

    m_running = true;
    while (m_running)
    {
        poll_incoming_messages();
        poll_connection_state_changes();
        std::this_thread::sleep_for(std::chrono::milliseconds(k_network_thread_sleep_ms));
    }

    m_interface->CloseConnection(m_connection, 0, nullptr, false);
    m_connection_status = connection_status_t::disconnected;

    m_interface = nullptr;

    // #NOTE is this ok with a client and server running in the same app?
    GameNetworkingSockets_Kill();
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

        m_data_received_callback_func(owning_buffer{ incoming_message->m_pData, static_cast<std::size_t>(incoming_message->m_cbSize) });

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
} // end namespace kb::network
