#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <zconf.h>
#include <fcntl.h>
#include <memory>
#include <arpa/inet.h>

#include "Server.h"
#include "constants.h"
#include "CorruptedRequestException.h"


auto main(int argc, char* argv[]) -> int {
    in_addr_t host = INADDR_ANY;
    int port = PORT_NO;
    int pLimit = PLAYER_LIMIT;
    int rLimit = ROOM_LIMIT;

    // parse args
    try {
        switch (argc) {
            case 5:  rLimit = std::stoi(argv[4]);
            case 4:  pLimit = std::stoi(argv[3]);
            case 3:  port = std::stoi(argv[2]);
            case 2:  host = inet_addr(argv[1]);
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

    // prepare socket
    int ssockfd  = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ssockfd == -1){
        std::cout << "Socket creation failed.\n";
        return 1;
    }
    fcntl(ssockfd, F_SETFL, O_NONBLOCK);

    // bind and listen
    struct sockaddr_in server{ AF_INET, htons(port), host };
    if (bind(ssockfd, (struct sockaddr *) &server, sizeof(server))){
        std::cerr << "Err: Socket bind failed.\n";
        return 2;
    }
    if (listen(ssockfd, 1) == -1){
       std::cerr << "Err: Socket listen failed.\n";
       return 3;
    }
    std::cout << "Socket is listening...\n";


    // prepare epoll
    struct epoll_event events[EPOLL_EVENT_MAX];
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1){
        std::cerr << "Err: Epoll create failed.\n";
        return 4;
    }

    // prepare Server instance
    auto serverInstance = std::make_unique<Server>(rLimit);
    int connections = 0;
    serverInstance->set_close_function([&connections](const int fd){
        close(fd);
        --connections;
    });

    while (true) {
        // accept clients until exhaustion
        while (connections < pLimit) {

            struct sockaddr_in client{};
            socklen_t client_len = sizeof(struct sockaddr_in);
            int csock_fd = accept(ssockfd, (struct sockaddr *) &client, &client_len);
            if (csock_fd == -1) {
                break;
            }
            std::cout << "Connection accepted\n";

            // create epoll event for accepted client
            struct epoll_event epoll_e{};
            epoll_e.events = EPOLLIN;
            epoll_e.data.fd = csock_fd;

            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, csock_fd, &epoll_e)) {
                std::cerr << "Err: Epoll ctl failed\n";
                close(csock_fd);
                continue;
            }

            ++connections;
        }

        // check for events
        int event_c = epoll_wait(epoll_fd, events, EPOLL_EVENT_MAX, EPOLL_TIMEOUT);
        if (event_c == -1){
            continue;
        }

        // handle events
        for (int i = 0; i < event_c; ++i) {
            serverInstance->notify(events[i].data.fd);
        }

        serverInstance->clean_up_rooms();
        // fixme clean up inactive players?
    }
}
