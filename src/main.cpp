#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <zconf.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string>

#include "Server.h"
#include "constants.h"
#include "networking_socket.hpp"


auto main(int argc, char* argv[]) -> int
{
    using namespace std::string_literals;

    auto host = "127.0.0.1"s;
    int port = PORT_NO;
    int pLimit = PLAYER_LIMIT;
    int rLimit = ROOM_LIMIT;

    // parse args
    try {
        switch (argc) {
            case 5:  rLimit = std::stoi(argv[4]);
            case 4:  pLimit = std::stoi(argv[3]);
            case 3:  port = std::stoi(argv[2]);
            case 2:  host = argv[1];
            case 1:  break;
            default: std::cerr << "Err: args not recognized\n";
                return 9;
        }
        if (!port || port > 65535) {
            std::cerr << "Invalid port\n";
            return 10;
        }
    } catch (std::exception& e) {
        std::cerr << "Err: failed to parse arguments\n";
        return 10;
    }

    Server(rLimit).start(host, port);
}
