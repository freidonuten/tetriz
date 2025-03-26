#include "Server.h"
#include "CorruptedRequestException.h"
#include "logger.hpp"
#include "networking_socket.hpp"
#include "protocol_structs.hpp"
#include "util.h"
#include "epoll.hpp"


void Server::notify(net::Socket<net::socket::client> client)
{
    context_.rebind(client.descriptor());

    const auto message = client.read_str();
    if (!message)
    {
        close(client);
        return;
    }

    try
    {
        log_trace("[{}] << {}", client.descriptor(), util::trim(*message));

        const auto request = protocol::deserialize(*message);
        const auto response = std::visit(dispatcher_, request).to_string();

        log_trace("[{}] >> {}", client.descriptor(), util::trim(response));

        client.write(response);

        // disconnect if login failed
        if (!context_.current_player_oid())
            close(client);
    }
    catch (const SerializerError& e)
    {
        log_trace("[{}] Corrupted request, closing client socket...", client.descriptor());
        close(client);

        return;
    }
}

void Server::close(net::Socket<net::socket::client> client)
{
    oid_mapper.erase(client.descriptor());
    client.close();
}

void Server::start()
{
    auto epoll = *make_epoll();
    auto socket = net::ServerSocket();
    socket.bind(config_.host, config_.port);
    socket.listen();

    log_info("Starting event loop");

    while (run)
    {
        for (const auto client : socket.accept())
            epoll.add(client);

        for (const auto client : epoll.wait())
            notify(client);
    }

    log_info("Event loop terminated");
}
