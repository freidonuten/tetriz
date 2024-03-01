#include <algorithm>
#include <variant>
#include <zconf.h>
#include <iostream>
#include "Server.h"
#include "Player.h"
#include "CorruptedRequestException.h"
#include "constants.h"
#include "Room.h"
#include "networking_socket.hpp"
#include "protocol_dispatch.hpp"
#include "protocol_structs.hpp"


Server::Server(int roomLimit)
    : roomLimit(roomLimit)
{ }

auto Server::create_player(const std::string& name) -> bool
{
    const auto name_matches = [name] (const auto& player) { return *player == name; };
    const auto player_iter = find_if(players.begin(), players.end(), name_matches);

    if (player_iter != end(players))
    {
        const auto& player = *player_iter;

        // reconnect player
        if (player->is_dead() || player->get_inactivity_ms() > ACTIVE_TIMEOUT_MS){
            player->connect(fd.descriptor());
            return true;
        }

        // if player is logged in, just nod him success
        return player->get_file_descriptor() == fd.descriptor();
    }

    // new player
    players.insert(std::make_shared<Player>(name, fd.descriptor()));

    return true;
}

void Server::notify(net::Socket<net::socket::client> client)
{
    const auto message = client.read();

    if (!message.has_value())
    {
        if (get_active_player())
        {
            get_active_player()->disconnect();
        }

        client.close();
        return;
    }

    // handle user request
    auto response = std::string{};

    try
    {
        std::cout << std::format("[{}] Received: {}", client.descriptor(), *message);

        response = std::visit(
            protocol::ProtocolDispatcher(*this),
            protocol::deserialize(*message)
        ).to_string();
    }
    catch (const SerializerError& e)
    {
        std::cout << std::format("[{}] Corrupted request, closing client socket...\n", client.descriptor());

        if (get_active_player())
        {
            get_active_player()->logout();
        }

        client.close();
    }

    // send response
    std::cout << "[" << client.descriptor() << "] Sending: " << response;
    client.write(response);

    // disconnect if login failed
    if (!get_active_player())
    {
        client.close();
    }
}

void Server::foreach_room(const std::function<void(const std::shared_ptr<Room>&)>& consumer)
{
    for (const auto& room: rooms)
    {
        consumer(room);
    }
}

void Server::foreach_player(const std::function<void(const std::shared_ptr<Player>&)>& consumer)
{
    for (const std::shared_ptr<Player>& player: players)
    {
        consumer(player);
    }
}

auto Server::join_room(unsigned room_id) -> bool
{
    // try to match player with file descriptor
    auto player = get_player_by_fd(fd.descriptor());
    if (!player)
    {
        return false;
    }

    // try to find room with such id
    auto room = get_room_by_id(room_id);
    if (!room || room->get_delta_t() < 0)
    {
        return false;
    }

    // try to join a return result
    return room->join(player);
}

auto Server::create_room(unsigned plimit) -> unsigned
{
    auto player = get_player_by_fd(fd.descriptor());

    if (player && roomLimit > rooms.size())
    {
        auto room = player->get_room().lock();
        if (room){
            return 0; // player already has a room
        }

        room = std::make_shared<Room>(plimit, player);
        rooms.insert(room);
        player->set_room(room);

        return room->get_id();
    }

    return 0;
}

auto Server::get_room_by_id(uint32_t room_id) const -> std::shared_ptr<Room>
{
    const auto id_matches = [&room_id](const auto& room){ return room->get_id() == room_id; };
    const auto room_iter = std::find_if(rooms.begin(), rooms.end(), id_matches);

    return room_iter == end(rooms) ? nullptr : *room_iter;
}



auto Server::get_player_by_fd(int32_t file_descriptor) const -> std::shared_ptr<Player>
{
    const auto fd_matches = [file_descriptor] (const auto& player) {
        return player->get_file_descriptor() == file_descriptor;
    };
    auto playerFound = find_if(players.begin(), players.end(), fd_matches);

    return playerFound == end(players) ? nullptr : *playerFound;
}

auto Server::get_active_room() const -> std::shared_ptr<Room>
{
    auto player = get_player_by_fd(fd.descriptor());
    return player ? player->get_room().lock() : nullptr;
}

auto Server::get_active_player() const -> std::shared_ptr<Player>
{
    return !active
        ? (active = get_player_by_fd(fd.descriptor()))
        : active;
}

auto Server::get_player_by_name(const std::string &name) const -> std::shared_ptr<Player>
{
    const auto name_matches = [&name](const auto& player){ return *player == name; };
    const auto player = std::find_if(players.begin(), players.end(), name_matches);

    return player == players.end() ? nullptr : *player;
}

void Server::clean_up_rooms()
{
    for (auto room_iter = rooms.begin(); room_iter != rooms.end(); )
    {
        auto room = *room_iter;

        if (!room->is_active())
        {
            const auto kick_player = [&room](const auto& player){
                if (player){ room->kick(player); }
            };

            room->foreach_player(kick_player);
            rooms.erase(room_iter++);

            std::cout << "[GC] room_" << room->get_id() << std::endl;

            continue;
        }

        ++room_iter;
    }
}

void Server::close_active()
{
    fd.close();
    connections--;
}

void Server::start(std::string_view host, int port)
{
    auto epoll = net::Epoll();
    auto socket = net::ServerSocket();
    socket.bind(host, port);
    socket.listen();

    while (true)
    {
        while (connections < connection_limit)
        {
            if (const auto client = socket.accept())
            {
                epoll.add(client.descriptor());
                continue;
            }

            break;
        }

        for (const auto descriptor : epoll.wait())
        {
            notify(descriptor);
        }

        clean_up_rooms();
    }
}
