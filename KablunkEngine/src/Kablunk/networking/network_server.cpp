#include "kablunkpch.h"

#include "Kablunk/networking/network_server.h"
#include "Kablunk/networking/net_var.h"
#include "Kablunk/networking/network_utils.h"

#include <array>

#include "Kablunk/networking/authentication.h"
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
    KB_CORE_INFO("[network_server]: Kicking client '{}'", p_client_id);
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
    const client_id_t p_client_id,
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

auto network_server::handle_client_authentication(
    client_info& p_client_info,
    const msgpack::object& p_auth_data_object
) noexcept -> void
{
    if (p_client_info.m_authenticated)
        return;

    const bool auth_check = check_client_auth_packet(p_client_info, p_auth_data_object);

    // disconnect client if they fail the auth check
    if (!auth_check)
    {
        kick_client(p_client_info.m_client_id);
        return;
    }

    p_client_info.m_authenticated = auth_check;
    send_authentication_response_to_client(p_client_info);
}

auto network_server::on_data_received(
    client_info& p_client_info,
    const msgpack::sbuffer& p_data_buffer
) noexcept -> void
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
    dispatch_handler_by_packet_type(packet_type, p_client_info, object);
}

auto network_server::dispatch_handler_by_packet_type(
    const underlying_packet_type_t p_packet_type,
    client_info& p_client_info,
    const msgpack::object& p_data_object
) noexcept -> void
{
    switch (p_packet_type)
    {
    case static_cast<underlying_packet_type_t>(packet_type::none):
    {
        KB_CORE_WARN("[network_server::on_data_received]: Invalid packet type 0!");
        break;
    }
    case static_cast<underlying_packet_type_t>(packet_type::kb_auth_check):
    {
        handle_client_authentication(p_client_info, p_data_object);
        break;
    }
    case static_cast<underlying_packet_type_t>(packet_type::kb_auth_response):
    {
        KB_CORE_WARN(
            "[network_server::on_data_received]: Unexpected packet type {} from client {}",
            p_packet_type,
            p_client_info.m_client_id
        );
        break;
    }
    case static_cast<underlying_packet_type_t>(packet_type::kb_rpc_call):
    {
        // #TODO handle response
        const auto rpc_response = m_rpc_dispatcher->dispatch(
            p_client_info,
            p_data_object
        );
        break;
    }
    case static_cast<underlying_packet_type_t>(packet_type::kb_rpc_response):
    {
        KB_CORE_WARN(
            "[network_server::on_data_received]: Unexpected packet type {} from client {}",
            p_packet_type,
            p_client_info.m_client_id
        );
        break;
    }
    default:
    {
        if (p_packet_type > static_cast<underlying_packet_type_t>(packet_type::kb_reserved))
            m_data_received_callback_func(p_client_info, p_data_object);
        else
        {
            KB_CORE_ERROR(
                "[network_server::on_data_received]: Unhandled kb internal packet type {}",
                p_packet_type
            );
            break;
        }
    }
    }
}

auto network_server::check_client_auth_packet(
    const client_info& p_client_info,
    const msgpack::object& p_data_object
) const noexcept -> bool
{
    const auto data_buffer_res = util::convert_object<authentication_check_data>(p_data_object);
    if (!data_buffer_res)
    {
        KB_CORE_WARN(
            "[network_server]: Tried authentication client '{}' but packet data was not invalid",
            p_client_info.m_client_id
        );
        return false;
    }

    const auto auth_data = data_buffer_res.value();
    const auto computed_hash = compute_auth_hash(
        auth_data.m_auth_version,
        std::string_view{ m_service_name }
    );
    const auto client_hash = auth_data.m_auth_hash;

    if (computed_hash == client_hash)
    {
        KB_CORE_INFO(
            "[network_server]: Successfully authenticated client '{}' with auth hash {}",
            p_client_info.m_client_id,
            auth_data.m_auth_hash
        );
        return true;
    }
    else
    {
        KB_CORE_INFO(
            "[network_server]: Failed to authenticate client '{}' with auth hash {}",
            p_client_info.m_client_id,
            auth_data.m_auth_hash
        );
        return false;
    }
}

auto network_server::disconnect_client(client_id_t p_client_id) noexcept -> void
{
    // locate client
    const auto it_client = m_connected_clients.find(p_client_id);
    if (it_client == m_connected_clients.end())
    {
        KB_CORE_WARN(
            "[network::network_server]: Trying to handle client disconnect but could not find client locally?"
        );
        return;
    }

    KB_CORE_INFO(
        "[network::network_server]: Disconnected client '{}'",
        p_client_id
    );

    // either ClosedByPeer or ProblemDetectedLocally - should be communicated to user callback
    m_client_disconnected_callback_func(it_client->second);

    m_connected_clients.erase(it_client);
}

auto network_server::send_authentication_response_to_client(const client_info& p_client_info) const noexcept -> void
{
    const auto auth_response = util::as_buffer(authentication_response_data{
        .m_packet_type = static_cast<underlying_packet_type_t>(packet_type::kb_auth_response),
        // #TODO require version in create call and pass...
        .m_service_version = "0.0.1",
        .m_client_id = p_client_info.m_client_id
    });

    send_packed_buffer_to_client(p_client_info.m_client_id, auth_response);
}

auto network_server::send_packed_buffer_to_all_clients(
    msgpack::sbuffer p_buffer,
    const client_id_t p_exclude_client,
    const bool p_reliable
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
    const client_id_t p_exclude_client,
    const bool p_reliable
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
    const i32 p_port,
    std::string p_service_name,
    callback_info&& p_callbacks
) noexcept -> ref<network_server>
{
    KB_CORE_ASSERT(p_port < std::numeric_limits<u16>::max(), "[network::network_server]: Port out of range!");

    return ref<network_server>::Create(
        p_port,
        std::move(p_service_name),
        std::forward<callback_info>(p_callbacks)
    );
}

network_server::network_server(
    const i32 port,
    std::string&& p_service_name,
    const callback_info& p_callbacks
) noexcept
    : m_port{ port }, m_service_name{ std::move(p_service_name) },
    m_data_received_callback_func{ p_callbacks.m_data_received_callback_func },
    m_client_connected_callback_func{ p_callbacks.m_client_connected_callback_func },
    m_client_disconnected_callback_func{ p_callbacks.m_client_disconnected_callback }
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
    config.SetPtr(
        k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
        // #FIXME clang-diagnostic-microsoft-cast conversion between pointer to function and pointer to object
        static_cast<void*>(&(network_server::connection_status_changed_callback))
    );

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

        KB_CORE_TRACE("[network_server]: Received message from client '{}'", it_client->first);

        if (incoming_message->m_cbSize)
        {
            msgpack::sbuffer buffer{};
            buffer.write(
                static_cast<const char*>(incoming_message->m_pData),
                static_cast<std::size_t>(incoming_message->m_cbSize)
            );

            auto& client_info = it_client->second;
            on_data_received(client_info, buffer);
        }

        incoming_message->Release();
    }
    //KB_CORE_TRACE("[network::network_server]: Finished polling messages.");
}

auto network_server::set_client_description(
    client_id_t p_connection,
    const std::string& p_description
) const noexcept -> void
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

auto network_server::on_connection_status_change(
    const SteamNetConnectionStatusChangedCallback_t* p_status
) noexcept -> void
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
            disconnect_client(p_status->m_hConn);
            break;
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
