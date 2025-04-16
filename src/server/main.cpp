#include <csignal>

#include "logger.hpp"
#include "epoll.hpp"

#include "server/room.hpp"


bool run = true;

void signal_handler(int signal)
{
    run = false;
}

Room room(3);

void notify(net::ConnectionWrapper client)
{
    if (const auto payload = client.read(); payload)
        room.notify(client, *payload);
    else
        room.leave(client);
}

auto main(int argc, char** argv) -> int
{
    log_level = Severity::Trace;
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
