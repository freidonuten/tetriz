#include <csignal>

#include "logger.hpp"
#include "epoll.hpp"

#include "server/room.hpp"
#include "server/room_list.hpp"



bool run = true;

void signal_handler(int signal)
{
    run = false;
}

RoomList rooms;

void notify(net::ConnectionWrapper client)
{
    const auto message = client
        .read()
        .and_then(tetriz::proto::deserialize);

    if (!message)
    {
        if (const auto room = rooms.get_room(client); room)
            room->get().leave(client);

        return;
    }

    if (message->type == tetriz::proto::MessageType::Hola)
    {
        rooms.get_available_room(std::get<tetriz::proto::DatagramHola>(message->payload).room_size)
            .notify(client, *message);

        return;
    }

    const auto room = rooms.get_room(client);

    if (!room)
        return;

    room->get().notify(client, *message);
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
