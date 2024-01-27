#pragma once

#include <memory>
#include <vector>
#include <set>

class Server;

class Protocol
{
public:
    Protocol(Server* server);

    auto handle(std::string& msg) -> std::string;
    auto handle_room_active(std::string &msg) -> std::string;
    auto handle_room_time(std::string &msg) -> std::string;
    auto handle_seed(std::string &msg) -> std::string;

private:
    auto handle_login(std::string& msg) -> std::string;
    auto handle_room_list(std::string& msg) -> std::string;
    auto handle_room_create(std::string& msg) -> std::string;
    auto handle_room_join(std::string& msg) -> std::string;
    auto handle_player_list(std::string& msg) -> std::string;
    auto handle_room(std::string& msg) -> std::string;
    auto handle_delta(std::string& msg) -> std::string;
    auto handle_move(std::string& msg) -> std::string;
    auto handle_move_get(std::string &msg) -> std::string;
    auto handle_move_last_timestamp(std::string &msg) -> std::string;
    auto handle_room_leave(std::string &msg) -> std::string;

    Server *server;

    inline static const std::set<std::string> valid_moves { "L", "R", "D", "SWP", "ROT", "DRP" };
};
