#include "Server.h"
#include "CorruptedRequestException.h"
#include "logger.hpp"
#include "networking_socket.hpp"
#include "protocol_dispatch.hpp"
#include "protocol_structs.hpp"
#include "util.h"


void Server::notify(net::Socket<net::socket::client> client)
{
    context_.rebind(client.descriptor());

    const auto message = client.read();
    if (!message)
    {
        client.close();
        return;
    }

    try
    {
        log_trace("[{}] << {}", client.descriptor(), util::trim(*message));

        const auto dispatcher = protocol::ProtocolDispatcher(model_, context_);
        const auto request = protocol::deserialize(*message);
        const auto response = std::visit(dispatcher, request).to_string();

        log_trace("[{}] >> {}", client.descriptor(), util::trim(response));

        client.write(response);

        // disconnect if login failed
        if (!context_.current_player_oid())
            client.close();
    }
    catch (const SerializerError& e)
    {
        log_trace("[{}] Corrupted request, closing client socket...", client.descriptor());
        client.close();

        return;
    }

}

void close(net::Socket<net::socket::client> client)
{
    oid_mapper.erase(client.descriptor());
    client.close();
}

void Server::start(std::string_view host, uint16_t port)
{
    auto epoll = net::Epoll();
    auto socket = net::ServerSocket();
    socket.bind(host, port);
    socket.listen();

    log_info("Starting event loop");

    while (run)
    {
        for (const auto client : socket.accept())
            epoll.add(client);

        for (const auto descriptor : epoll.wait())
            notify(descriptor);
    }

    log_info("Event loop terminated");
}
