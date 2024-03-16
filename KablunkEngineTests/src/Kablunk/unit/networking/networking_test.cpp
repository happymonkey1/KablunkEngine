#include "Kablunk/Core/Core.h"

#include "Kablunk/networking/network_server.h"
#include "Kablunk/networking/network_client.h"

#include <catch_amalgamated.hpp>

using namespace kb;

TEST_CASE("network initialization succeeds", "[networking]")
{
    constexpr u32 k_delay_ms = 250;

    auto data_callback_func = [](
        const network::client_info& p_client_info,
        const msgpack::sbuffer& p_data_buffer
        ) -> void
        {
            KB_CORE_INFO("[networking_test]: data recieved callback called!");
        };
    auto client_connected_func = [](
        const network::client_info& p_client_info
        ) -> void
        {
            KB_CORE_INFO("[networking_test]: client connected callback called!");
        };
    auto client_disconnected_func = [](
        const network::client_info& p_client_info
        ) -> void
        {
            KB_CORE_INFO("[networking_test]: client disconnected callback called!");
        };

    u32 port = 20420;
    auto server = network::network_server::create(
        port,
        data_callback_func,
        client_connected_func,
        client_disconnected_func
    );
    server->bind_rpc(
        "add",
        [](int a, int b) -> int
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
    server->start();

    std::this_thread::sleep_for(
        std::chrono::milliseconds(k_delay_ms)
    );

    auto client = network::network_client::create();
    client->connect_to_server(fmt::format("127.0.0.1:{}", port));

    // #TODO: use `wait_for_connection` function
    std::this_thread::sleep_for(
        std::chrono::milliseconds(k_delay_ms)
    );
    client->call_rpc("add", 2, 3);

    // #TODO: require on response when that is implemented
    std::this_thread::sleep_for(
        std::chrono::milliseconds(k_delay_ms)
    );
    //REQUIRE()

    client->disconnect();

    server->stop();
}
