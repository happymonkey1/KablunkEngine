#include "Kablunk/Core/Core.h"

#include "Kablunk/networking/network_server.h"
#include "Kablunk/networking/network_client.h"

#include <catch_amalgamated.hpp>

using namespace kb;

TEST_CASE("network initialization succeeds", "[networking]")
{
    auto data_callback_func = [](
        const network::client_info& p_client_info,
        const msgpack::sbuffer& p_data_buffer
        ) -> void
        {
            KB_CORE_INFO("data recieved callback called!");
        };
    auto client_connected_func = [](
        const network::client_info& p_client_info
        ) -> void
        {
            KB_CORE_INFO("client connected callback called!");
        };
    auto client_disconnected_func = [](
        const network::client_info& p_client_info
        ) -> void
        {
            KB_CORE_INFO("client disconnected callback called!");
        };

    auto server = network::network_server::create(
        20420,
        data_callback_func,
        client_connected_func,
        client_disconnected_func
    );
    /*server->bind_rpc(
        "add",
        [](int a, int b) -> int { return a + b; }
    );*/
    server->start();

    std::this_thread::sleep_for(
        std::chrono::milliseconds(1000)
    );

    auto client = network::network_client::create();
    client->connect_to_server("127.0.0.1");

    // #TODO: use `wait_for_connection` function
    std::this_thread::sleep_for(
        std::chrono::milliseconds(1000)
    );
    //client->call_rpc("add", 2, 3);

    // #TODO: require on response when that is implemented
    //REQUIRE()

    client->disconnect();

    server->stop();
}
