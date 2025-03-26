#include <cassert>

#include "epoll.hpp"
#include "logger.hpp"
#include "networking_socket.hpp"


Epoll::Epoll(int32_t descriptor)
    : descriptor_(descriptor)
{
    assert(descriptor_ != net::invalid_descriptor);
}

Epoll::~Epoll()
{
    if (descriptor_ != net::invalid_descriptor)
        close(descriptor_);
}

bool Epoll::add(int32_t observed_fd)
{
    auto epoll_e = epoll_event{
        .events = EPOLLIN,
        .data = epoll_data{ .fd = observed_fd }
    };

    fcntl(observed_fd, F_SETFL, O_NONBLOCK);

    if (epoll_ctl(descriptor_, EPOLL_CTL_ADD, observed_fd, &epoll_e) != 0)
    {
        log_warning("Failed to add descriptor {} to epoll instance {}", observed_fd, descriptor_);
        return false;
    }

    return true;
}

auto Epoll::descriptor() const -> int32_t
{
    return descriptor_;
}

auto make_epoll() -> std::optional<Epoll>
{
    const auto epoll_fd = epoll_create1(0);

    if (epoll_fd != net::invalid_descriptor)
        return Epoll(epoll_fd);

    return {};
}
