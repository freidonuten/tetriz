#pragma once

#include "Context.hpp"
#include "configuration.h"
#include "networking_socket.hpp"
#include "protocol_dispatch.hpp"


class Server
{
public:
    Server(const Configuration& config, protocol::ProtocolDispatcher& dispatcher, Context& context)
        : config_(config)
        , dispatcher_(dispatcher)
        , context_(context)
    {}

    void start();

private:
    const Configuration& config_;
    protocol::ProtocolDispatcher& dispatcher_;
    Context& context_;
    bool run = true;

    void close(net::Socket<net::socket::client> client);
    void notify(net::Socket<net::socket::client> client);
};
