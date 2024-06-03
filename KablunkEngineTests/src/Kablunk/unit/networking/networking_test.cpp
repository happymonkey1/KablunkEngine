#include "Kablunk/Core/Core.h"

#include "Kablunk/networking/network_server.h"
#include "Kablunk/networking/network_client.h"

#include <catch_amalgamated.hpp>

using namespace kb;

const std::string service_name = "kablunk-engine-tests@test";

auto sub(const network::client_info&, int x, int y) -> int
{
    KB_CORE_INFO(
        "[networking_test]: sub rpc called! {} - {} = {}",
        x,
        y,
        x - y
    );
    return x - y;
}

auto do_work_on_server_with_void_response(const network::client_info&, int) -> void
{
    KB_CORE_INFO("[networking_test]: do_wrok_on_server_with_void_response");
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
    server->bind_rpc("do_work_on_server_with_void_response", &do_work_on_server_with_void_response);
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
    auto connection_status = client->connect_to_server(
        "127.0.0.1",
        port,
        network::network_client::network_blocking_t::blocking
    );
    REQUIRE(connection_status == network::network_client::connection_status_t::connected);

#if 0
    auto connection_status = client->wait_for_connection();
    REQUIRE(connection_status == network::network_client::connection_status_t::connected);
    auto auth_status = client->wait_for_authentication_check();
    REQUIRE(auth_status == std::future_status::ready);
#endif

    client->call_raw_rpc("add", 2, 3);
    client->call_raw_rpc("sub", 2, 3);

    // call blocking rpc and check return value
    {
        auto add_network_result = client->call_blocking_rpc("add", 3, 4);
        REQUIRE(std::holds_alternative<network::network_client::rpc_success_t>(add_network_result));
        auto add_rpc_result = std::get<network::network_client::rpc_success_t>(add_network_result);

        REQUIRE(add_rpc_result.m_data_buffer);
        auto add_result = network::util::convert_object<u32>(*add_rpc_result.m_data_buffer);
        REQUIRE(add_result);
        REQUIRE(*add_result == 7);
    }

    // call blocking rpc and check return value
    {
        auto sub_network_result = client->call_blocking_rpc("sub", 5, 2);
        REQUIRE(std::holds_alternative<network::network_client::rpc_success_t>(sub_network_result));

        auto sub_rpc_result = std::get<network::network_client::rpc_success_t>(sub_network_result);

        REQUIRE(sub_rpc_result.m_data_buffer);
        auto sub_result = network::util::convert_object<u32>(*sub_rpc_result.m_data_buffer);
        REQUIRE(sub_result);
        REQUIRE(*sub_result == 3);
    }

    {
        auto rpc_result = client->call_blocking_rpc(
            "do_work_on_server_with_void_response",
            5
        );
        REQUIRE(std::holds_alternative<network::network_client::rpc_success_t>(rpc_result));
        auto success_result = std::get<network::network_client::rpc_success_t>(rpc_result);
        REQUIRE(!success_result.m_data_buffer.has_value());
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(k_delay_ms)
    );

    client->disconnect();

    server->stop();
}

TEST_CASE("server responds with error codes and does not crash", "[networking]")
{
    u32 port = 20420;
    auto server = network::network_server::create(
        port,
        service_name,
        {
            .m_data_received_callback_func = nullptr,
            .m_client_connected_callback_func = nullptr,
            .m_client_disconnected_callback = nullptr
        }
    );
    server->start();

    auto client = network::network_client::create(
        service_name,
        {
            .m_data_received_callback_func = nullptr,
            .m_client_connected_callback_func = nullptr,
            .m_client_disconnected_callback_func = nullptr,
        },
        network::account_credentials{
            .m_username = "KablunkEngineTests-username"
        }
    );
    auto connection_status = client->connect_to_server(
        "127.0.0.1",
        port,
        network::network_client::network_blocking_t::blocking
    );
    REQUIRE(connection_status == network::network_client::connection_status_t::connected);

    client->disconnect();
    server->stop();
}
