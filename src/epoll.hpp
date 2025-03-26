#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <sys/epoll.h>

#include "constants.h"
#include "networking_socket.hpp"


class Epoll
{
public:
    ~Epoll();

    Epoll(const Epoll& other) = delete;
    Epoll(Epoll&& other) { std::swap(descriptor_, other.descriptor_); }

    Epoll& operator=(const Epoll& other) = delete;
    Epoll& operator=(Epoll&& other) { std::swap(descriptor_, other.descriptor_); return *this; }

    bool add(int32_t observed_fd);
    //bool add(const auto& resource) { return resource.descriptor(); }

    [[nodiscard]] auto descriptor() const -> int32_t;
    [[nodiscard]] auto wait() -> decltype(auto)
    {
        constexpr auto event_to_descriptor = [](const auto& event)
            -> net::Socket<net::socket::client> { return event.data.fd; };
        const auto count = std::max(0,
            epoll_wait(descriptor_, events_.data(), EPOLL_EVENT_MAX, EPOLL_TIMEOUT));

        return events_
            | std::views::take(count)
            | std::views::transform(event_to_descriptor);
    }
    
    friend auto make_epoll() -> std::optional<Epoll>;

private:
    int32_t descriptor_ = -1;
    std::array<epoll_event, EPOLL_EVENT_MAX> events_{};

    Epoll(int32_t descriptor);
};

auto make_epoll() -> std::optional<Epoll>;
