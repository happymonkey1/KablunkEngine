#include "Kablunk/Core/Core.h"

#include "Kablunk/networking/network_server.h"
#include "Kablunk/networking/network_client.h"

#include <catch_amalgamated.hpp>

using namespace kb;

auto sub(const network::client_info&, int x, int y) -> int
{
    KB_CORE_INFO("sub called!");
    return x - y;
}

TEST_CASE("network initialization succeeds", "[networking]")
{
    constexpr u32 k_delay_ms = 250;

    auto data_callback_func = [](
        const network::client_info& p_client_info,
        const msgpack::object& p_data_object
        ) -> void
        {
            KB_CORE_INFO("[networking_test]: server's data recieved callback called!");
        };
    auto client_connected_func = [](
        const network::client_info& p_client_info
        ) -> void
        {
            KB_CORE_INFO("[networking_test]: server's client connected callback called!");
        };
    auto client_disconnected_func = [](
        const network::client_info& p_client_info
        ) -> void
        {
            KB_CORE_INFO("[networking_test]: server's client disconnected callback called!");
        };

    auto client_data_callback_func = [](
        const msgpack::object& p_data_object
        ) -> void
        {
            KB_CORE_INFO("[networking_test]: client's data recieved callback called!");
        };
    auto client_client_connected_func = [](
        ) -> void
        {
            KB_CORE_INFO("[networking_test]: client's client connected callback called!");
        };
    auto client_client_disconnected_func = [](
        ) -> void
        {
            KB_CORE_INFO("[networking_test]: client's client disconnected callback called!");
        };

    const std::string service_name = "kablunk-engine-tests@test";

    u32 port = 20420;
    auto server = network::network_server::create(
        port,
        service_name,
        {
            .m_data_received_callback_func = data_callback_func,
            .m_client_connected_callback_func = client_connected_func,
            .m_client_disconnected_callback = client_disconnected_func
        }
    );
    server->bind_rpc(
        "add",
        [](const network::client_info& p_client_info, int a, int b) -> int
        {
            KB_CORE_INFO(
                "[networking_test]: add rpc called! {} + {} = {}",
                a,
                b,
                a + b
            );
            return a + b;
        }
    );
    server->bind_rpc(
        "sub",
        &sub
    );
    server->start();

    std::this_thread::sleep_for(
        std::chrono::milliseconds(k_delay_ms)
    );

    auto client = network::network_client::create(
        service_name,
        {
            .m_data_received_callback_func = client_data_callback_func,
            .m_client_connected_callback_func = client_client_connected_func,
            .m_client_disconnected_callback_func = client_client_disconnected_func,
        },
        network::account_credentials{
            .m_username = "KablunkEngineTests-username"
        }
    );
    client->connect_to_server("127.0.0.1", port);

    // #TODO: use `wait_for_connection` function
    std::this_thread::sleep_for(
        std::chrono::milliseconds(k_delay_ms)
    );
    client->call_rpc("add", 2, 3);
    client->call_rpc("sub", 2, 3);

    // #TODO: require on response when that is implemented
    std::this_thread::sleep_for(
        std::chrono::milliseconds(k_delay_ms)
    );
    //REQUIRE()

    client->disconnect();

    server->stop();
}
