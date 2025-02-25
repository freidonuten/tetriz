#pragma once

#include "Context.hpp"
#include "Model.hpp"
#include "networking_socket.hpp"


class Server
{
public:
    Server() = default;

    void start(std::string_view host, uint16_t port);

private:
    Model model_{};
    Context context_{model_};
    bool run = true;

    void close(net::Socket<net::socket::client> client);
    void notify(net::Socket<net::socket::client> client);
};
