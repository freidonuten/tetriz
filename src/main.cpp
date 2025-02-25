#include <netinet/in.h>
#include <sys/epoll.h>
#include <zconf.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string>

#include "Server.h"
#include "constants.h"
#include "logger.hpp"


auto main(int argc, char** argv) -> int
{
    using namespace std::string_literals;

    auto host = "127.0.0.1"s;
    auto port = PORT_NO;

    try {
        switch (argc) {
            case 3:  port = std::stoi(argv[2]);
                     [[fallthrough]];
            case 2:  host = argv[1];
                     [[fallthrough]];
            case 1:  break;
            default:
                log_error("args not recognized");
                return 9;
        }
        if (!port || port > 65535) {
            log_error("Invalid port");
            return 10;
        }
    } catch (std::exception& e) {
        log_error("Err: failed to parse arguments");
        return 10;
    }

    auto server = Server{};

    server.start(host, port);

    return 0;
}
