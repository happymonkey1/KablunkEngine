#include "kablunkpch.h"

#include "Kablunk/networking/network_server.h"
#include "Kablunk/networking/net_var.h"

#include <array>

#include "Kablunk/networking/game_networking_sockets.h"

namespace kb::network
{ // start namespace kb::network

constexpr size_t k_max_servers = 8ull;
static std::size_t s_registered_server_count = 0ull;
static std::array<network_server*, k_max_servers> s_registered_servers{};

namespace details
{ // start namespace ::details

static auto register_server_for_connection_callback(network_server* p_server_ptr) -> void
{
    KB_CORE_ASSERT(s_registered_server_count < k_max_servers, "[network_server]: exceeded maximum server count!");
    s_registered_servers[s_registered_server_count++] = p_server_ptr;
}

static auto unregister_server_for_connection_callback(network_server* p_server_ptr) -> void
{
    KB_CORE_ASSERT(s_registered_server_count > 0, "[network_server]: Trying to unregister a server when there are none in the registered server list?");

    std::size_t server_index = 0ull;
    for (const auto server_ptr : s_registered_servers)
    {
        if (server_ptr == p_server_ptr)
            break;

        ++server_index;
    }

    network_server* tail = s_registered_servers[s_registered_server_count - 1];
    s_registered_servers[server_index] = tail;
    s_registered_servers[s_registered_server_count - 1] = nullptr;

    s_registered_server_count -= 1;
}

} // end namespace ::details

network_server::~network_server() noexcept
{
    KB_CORE_INFO("[network::network_server]: Destroying network server.");
    if (m_network_thread.joinable())
    {
        KB_CORE_TRACE("[network::network_server]: Joining network loop thread.");
        m_network_thread.join();
    }
}

auto network_server::start() noexcept -> void
{
    if (m_running)
        return;

    init_game_networking_sockets_lib();

    KB_CORE_ASSERT(
        m_client_connected_callback_func,
        "[network::network_server]: client connected callback is not set!"
    );
    KB_CORE_ASSERT(
        m_client_disconnected_callback_func,
        "[network::network_server]: client disconnected callback is not set!"
    );
    KB_CORE_ASSERT(
        m_data_received_callback_func,
        "[network::network_server]: data received callback is not set!"
    );

    m_network_thread = std::thread([this] { network_loop(); });
}

auto network_server::stop() noexcept -> void
{
    KB_CORE_INFO("[network_server]: Stopping server.");
    m_running = false;
    details::unregister_server_for_connection_callback(this);
}

auto network_server::kick_client(client_id_t p_client_id) noexcept -> void
{
    m_interface->CloseConnection(p_client_id, 0, "Kicked by host", false);
}

#if 0
auto network_server::send_buffer_to_client(client_id_t p_client_id, owning_buffer p_buffer, bool p_reliable) noexcept -> void
{
    auto result = m_interface->SendMessageToConnection(
        p_client_id,
        p_buffer.get(),
        static_cast<u32>(p_buffer.size()),
        p_reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable,
        nullptr
    );
}

auto network_server::send_buffer_to_all_clients(
    owning_buffer p_buffer,
    client_id_t p_exclude_client,
    bool p_reliable
) noexcept -> void
{
    for (const auto& [client_id, client_info] : m_connected_clients)
    {
        if (client_id == p_exclude_client)
            continue;

        send_buffer_to_client(client_id, p_buffer, p_reliable);
    }
}
#endif

auto network_server::send(
    client_id_t p_client_id,
    const void* p_data,
    size_t p_size,
    bool p_reliable /*= true*/
) const noexcept -> void
{
    // #TODO check result
    auto result = m_interface->SendMessageToConnection(
        p_client_id,
        p_data,
        static_cast<u32>(p_size),
        p_reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable,
        nullptr
    );
}

auto network_server::send_packed_buffer_to_all_clients(
    msgpack::sbuffer p_buffer,
    client_id_t p_exclude_client,
    bool p_reliable
) const noexcept -> void
{
    for (const auto& [client_id, client_info] : m_connected_clients)
    {
        if (client_id == p_exclude_client)
            continue;

        send(
            client_id,
            p_buffer.data(),
            p_buffer.size(),
            p_reliable
        );
    }
}

auto network_server::send_packed_buffer_to_all_clients(
    const ref<msgpack::sbuffer>& p_buffer_ref,
    client_id_t p_exclude_client,
    bool p_reliable
) const noexcept -> void
{
    KB_CORE_ASSERT(
        p_buffer_ref,
        "[network::network_server]: Trying to send buffer to all clients but buffer is null?"
    );

    for (const auto& [client_id, client_info] : m_connected_clients)
    {
        if (client_id == p_exclude_client)
            continue;

        send(
            client_id,
            p_buffer_ref->data(),
            p_buffer_ref->size(),
            p_reliable
        );
    }
}

auto network_server::create(
    i32 p_port,
    data_received_callback_func_t p_data_received_callback_func,
    client_connected_callback_func_t p_client_connected_callback_func,
    client_disconnected_callback_func_t p_client_disconnected_callback
) noexcept -> ref<network_server>
{
    KB_CORE_ASSERT(p_port < std::numeric_limits<u16>::max(), "[network::network_server]: Port out of range!");

    return ref<network_server>::Create(
        p_port,
        p_data_received_callback_func,
        p_client_connected_callback_func,
        p_client_disconnected_callback
    );
}

network_server::network_server(
    i32 port,
    data_received_callback_func_t p_data_received_callback_func,
    client_connected_callback_func_t p_client_connected_callback_func,
    client_disconnected_callback_func_t p_client_disconnected_callback
) noexcept
    : m_port{ port }, m_data_received_callback_func{ p_data_received_callback_func },
    m_client_connected_callback_func{ p_client_connected_callback_func },
    m_client_disconnected_callback_func{ p_client_disconnected_callback }
{
    details::register_server_for_connection_callback(this);
}

auto network_server::network_loop() noexcept -> void
{
    KB_CORE_INFO("[network::network_server]: Starting network loop.");
    m_running = true;

    m_interface = SteamNetworkingSockets();

    SteamNetworkingIPAddr server_local_address{};
    server_local_address.Clear();
    server_local_address.m_port = static_cast<u16>(m_port);

    SteamNetworkingConfigValue_t config{};
    // must correspond to the number of config values set below
    constexpr i32 k_options_count = 1;
    config.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, static_cast<void*>(&network_server::connection_status_changed_callback));

    // start listening socket
    m_listen_socket = m_interface->CreateListenSocketIP(server_local_address, k_options_count, &config);

    if (m_listen_socket == k_HSteamListenSocket_Invalid)
    {
        on_fatal_error(fmt::format("Failed to listen on port {}", m_port));
        return;
    }

    // #TODO should be optional
    m_poll_group = m_interface->CreatePollGroup();
    if (m_poll_group == k_HSteamNetPollGroup_Invalid)
    {
        on_fatal_error(fmt::format("Failed to create poll group."));
        return;
    }

    KB_CORE_INFO("[network::network_server]: Server listening on port {}", m_port);

    while (m_running)
    {
        poll_incoming_messages();
        poll_connection_state_changes();
        std::this_thread::sleep_for(std::chrono::milliseconds(k_network_thread_sleep_ms));
    }

    KB_CORE_INFO("[network::network_server]: Finished blocking network thread.");
    KB_CORE_INFO("[network::network_server]: Closing all open connections ({})", m_connected_clients.size());
    for (const auto& [client_id, client_info] : m_connected_clients)
    {
        m_interface->CloseConnection(
            client_id,
            0,
            "Server Shutdown",
            true
        );
        KB_CORE_TRACE("[network::network_server]: Closing connect for client_id {}", client_id);
    }

    m_connected_clients.clear();

    KB_CORE_ASSERT(
        m_interface->DestroyPollGroup(m_poll_group),
        "[network::network_server]: Failed to destroy poll group. Poll group was invalid?"
    );
    m_poll_group = k_HSteamNetPollGroup_Invalid;

    KB_CORE_ASSERT(
        m_interface->CloseListenSocket(m_listen_socket),
        "[network::network_server]: Failed to destroy listen socket!"
    );
    m_listen_socket = k_HSteamListenSocket_Invalid;

    m_interface = nullptr;

    kill_game_networking_sockets_lib();
    KB_CORE_INFO("[network::network_server]: Network loop cleanup complete.");
}

auto network_server::poll_incoming_messages() noexcept -> void
{
    //KB_CORE_TRACE("[network::network_server]: Polling messages...");
    while (m_running)
    {
        constexpr i32 k_max_message = 1ull;
        ISteamNetworkingMessage* incoming_message = nullptr;
        const i32 message_count = m_interface->ReceiveMessagesOnPollGroup(m_poll_group, &incoming_message, k_max_message);
        if (message_count == 0)
            break;

        if (message_count < 0)
        {
            m_running = false;
            break;
        }

        auto it_client = m_connected_clients.find(incoming_message->m_conn);
        if (it_client == m_connected_clients.end())
        {
            KB_CORE_WARN("[network_server]: Received message from an unregistered client!");
            continue;
        }

        if (incoming_message->m_cbSize)
        {
            msgpack::sbuffer buffer{};
            buffer.write(
                static_cast<const char*>(incoming_message->m_pData),
                static_cast<std::size_t>(incoming_message->m_cbSize)
            );

            m_data_received_callback_func(it_client->second,buffer);
        }

        incoming_message->Release();
    }
    //KB_CORE_TRACE("[network::network_server]: Finished polling messages.");
}

auto network_server::set_client_description(client_id_t p_connection, const std::string& p_description) noexcept -> void
{
    m_interface->SetConnectionName(p_connection, p_description.c_str());
}

auto network_server::connection_status_changed_callback(
    const SteamNetConnectionStatusChangedCallback_t* p_info
) noexcept -> void
{
    for (size_t i = 0; i < s_registered_server_count; ++i)
    {
        const auto server = s_registered_servers[i];
        server->on_connection_status_change(p_info);
    }
}

auto network_server::on_connection_status_change(const SteamNetConnectionStatusChangedCallback_t* p_status) noexcept -> void
{
    switch (p_status->m_info.m_eState)
    {
    case k_ESteamNetworkingConnectionState_None:
        // #NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
        break;
    case k_ESteamNetworkingConnectionState_ClosedByPeer: [[fallthrough]];
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
    {
        // ignore if they were not previously connected (if they disconnected before we accepted the connection).
        if (p_status->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
        {
            // locate the client. Note that it should have been found, because this
            // is the only codepath where we remove clients (except on shutdown),
            // and connection change callbacks are dispatched in queue order.
            const auto it_client = m_connected_clients.find(p_status->m_hConn);
            //assert(itClient != m_mapClients.end());
            if (it_client == m_connected_clients.end())
            {
                KB_CORE_WARN(
                    "[network::network_server]: Trying to handle client disconnect but could not find client locally?"
                );
                break;
            }

            KB_CORE_INFO(
                "[network::network_server]: client '{}' disconnected",
                it_client->second.m_client_id
            );

            // either ClosedByPeer or ProblemDetectedLocally - should be communicated to user callback
            m_client_disconnected_callback_func(it_client->second);

            m_connected_clients.erase(it_client);
        }

        // clean up the connection.  This is important!
        // the connection is "closed" in the network sense, but
        // it has not been destroyed.  We must close it on our end, too
        // to finish up.  The reason information do not matter in this case,
        // and we cannot linger because it's already closed on the other end,
        // so we just pass 0s.
        m_interface->CloseConnection(p_status->m_hConn, 0, nullptr, false);
        break;
    }

    case k_ESteamNetworkingConnectionState_Connecting:
    {
        // accept connection
        if (m_interface->AcceptConnection(p_status->m_hConn) != k_EResultOK)
        {
            m_interface->CloseConnection(p_status->m_hConn, 0, nullptr, false);
            KB_CORE_WARN("Couldn't accept connection (it was already closed?)");
            break;
        }

        // assign the poll group
        if (!m_interface->SetConnectionPollGroup(p_status->m_hConn, m_poll_group))
        {
            m_interface->CloseConnection(p_status->m_hConn, 0, nullptr, false);
            KB_CORE_ERROR("Failed to set poll group");
            break;
        }

        // retrieve connection info
        SteamNetConnectionInfo_t connection_info;
        m_interface->GetConnectionInfo(p_status->m_hConn, &connection_info);

        // register connected client
        auto& client = m_connected_clients[p_status->m_hConn];
        client.m_client_id = static_cast<client_id_t>(p_status->m_hConn);
        client.m_connection_description = connection_info.m_szConnectionDescription;

        KB_CORE_INFO("[network::network_server]: Accepted connection from client '{}'", client.m_client_id);

        // user callback
        m_client_connected_callback_func(client);

        break;
    }

    case k_ESteamNetworkingConnectionState_Connected:
        // we will get a callback immediately after accepting the connection.
        // since we are the server, we can ignore this, it's not news to us.
        break;

    default:
        break;
    }
}

auto network_server::poll_connection_state_changes() -> void
{
    m_interface->RunCallbacks();
}

auto network_server::on_fatal_error(const std::string& p_message) -> void
{
    KB_CORE_ERROR("[network_server]: Fatal Error! Message='{}'", p_message);
}


} // end namespace kb::network
