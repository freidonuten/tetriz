#include <csignal>
#include <map>

#include "logger.hpp"
#include "epoll.hpp"

#include "game_engine.hpp"
#include "server/room.hpp"


bool run = true;

void signal_handler(int signal)
{
    run = false;
}

std::map<int32_t, GameEngine> games;
Room room;


void notify(net::ConnectionWrapper client)
{
    log_info("Received...");

    if (const auto payload = client.read(); payload)
    {
        if (!games.contains(client.descriptor()))
        {
            log_info("Opening...");
            room.add_player(client);
        }

        log_info("Notifying...");
        room.notify(client, *payload);
    }
    else
    {
        log_info("Closing...");
        games.erase(client.descriptor());
        client.close();
    }
}

auto main(int argc, char** argv) -> int
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    auto epoll = make_epoll();
    auto socket = net::ServerSocket();
    socket.set_non_blocking();
    socket.bind("127.0.0.1", 6666);
    socket.listen();

    log_info("Starting event loop");

    while (run)
    {
        for (auto client : socket.accept())
        {
            net::Socket(client).set_nodelay();
            epoll->add(client);
        }

        for (const auto client : epoll->wait())
            notify(client);
    }
}
