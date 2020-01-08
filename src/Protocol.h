//
// Created by martin on 03.01.20.
//

#ifndef UNTITLED_PROTOCOL_H
#define UNTITLED_PROTOCOL_H

#include <memory>
#include <vector>
#include <set>
#include "Server.h"

class Server;
class Protocol {

private:
    const std::set<std::string> validMoves { "L", "R", "D", "SWP", "ROT", "DRP" };

    class Server *server;

    std::string handle_login(std::string& msg);
    std::string handle_room_list(std::string& msg);
    std::string handle_room_create(std::string& msg);
    std::string handle_room_join(std::string& msg);
    std::string handle_player_list(std::string& msg);
    std::string handle_room(std::string& msg);
    std::string handle_delta(std::string& msg);
    std::string handle_move(std::string& msg);

public:
    Protocol(Server* server);
    std::string handle(std::string& msg);

    std::string handle_move_get(std::string &basicString);
};



#endif //UNTITLED_PROTOCOL_H
