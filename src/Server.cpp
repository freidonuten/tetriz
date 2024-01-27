#include <algorithm>
#include <zconf.h>
#include <iostream>
#include "Server.h"
#include "Player.h"
#include "Protocol.h"
#include "CorruptedRequestException.h"
#include "constants.h"
#include "Room.h"


Server::Server(int roomLimit)
    : protocol(new Protocol(this)), roomLimit(roomLimit)
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
            player->connect(fd);
            return true;
        }

        // if player is logged in, just nod him success
        return player->get_file_descriptor() == fd;
    }

    // new player
    players.insert(std::make_shared<Player>(name, fd));

    return true;
}

void Server::notify(int fileDescriptor)
{
    active = nullptr;
    fd = fileDescriptor;
    auto message = std::string("");

    // should feed msg until buffer stops overflowing
    while (true)
    {
        auto chunk = std::array<char, 17>{0};
        int result = read(fileDescriptor, chunk.data(), chunk.size());
        message += chunk.data(); // FIXME read directly into message string

        if (result == 0)
        {
            if (get_active_player())
            {
                get_active_player()->disconnect();
            }

            close(fd);
            return;
        }

        if (result != chunk.size() || message[message.length() - 1] == MSG_SEP)
        {
            break; // did not fill the buffer
        }
    }

    // handle user request
    auto response = std::string{};

    try
    {
        std::cout << "[" << fd << "] Received: " << message;
        response = protocol->handle(message);
    }
    catch (const CorruptedRequestException& e)
    {
        std::cout << "[" << fd << "] Corrupted request, closing client socket...\n";

        if (get_active_player())
        {
            get_active_player()->logout();
        }

        close(fd);
    }

    // send response
    std::cout << "[" << fd << "] Sending: " << response;
    write(fileDescriptor, response.data(), response.length());

    // disconnect if login failed
    if (!get_active_player())
    {
        close(fd);
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
    auto player = get_player_by_fd(fd);
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
    auto player = get_player_by_fd(fd);

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
    auto player = get_player_by_fd(fd);
    return player ? player->get_room().lock() : nullptr;
}

auto Server::get_active_player() const -> std::shared_ptr<Player>
{
    return !active
        ? (active = get_player_by_fd(fd))
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
    close(fd);
}

void Server::set_close_function(std::function<void(const int)> closeFunc)
{
    close = std::move(closeFunc);
}
