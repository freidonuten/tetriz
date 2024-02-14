#pragma once


#include "constants.h"
#include "crtp.hpp"
#include <ranges>
#include <cstdint>
#include <array>
#include <netinet/in.h>
#include <optional>
#include <string_view>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>


namespace net
{
    constexpr auto invalid_descriptor = -1;

    namespace socket
    {
        namespace detail
        {
            template <typename Host>
            struct socket_policy : crtp_base<Host>
            {
            protected:
                socket_policy() = default;
                
                [[nodiscard]] constexpr
                auto fd() const
                { return this->self().descriptor(); }
            };
        }

        template <typename Host>
        struct server : public detail::socket_policy<Host>
        {
            void bind(std::string_view address, uint16_t port)
            {
                auto host = in_addr_t{ inet_addr(address.data()) };
                auto server = sockaddr_in{ AF_INET, htons(port), INADDR_ANY/*host*/ };

                if (::bind(this->fd(), reinterpret_cast<sockaddr*>(&server), sizeof(server)) != 0)
                {
                    throw "Failed to bind socket";
                }
            }

            void listen()
            {
                if (::listen(this->fd(), 1) == -1)
                {
                    throw "Socket listen failed";
                }
            }

            [[nodiscard]]
            auto accept() -> int32_t
            {
                struct sockaddr_in client{};
                socklen_t client_len = sizeof(struct sockaddr_in);
                return ::accept(this->fd(), (struct sockaddr *) &client, &client_len);
            }
        };

        template <typename Host>
        struct client : public detail::socket_policy<Host>
        {
            void connect(std::string_view address, uint16_t port) const
            {
                auto host = in_addr_t{ inet_addr(address.data()) };
                auto server = sockaddr_in{ AF_INET, htons(port), host };

                if (::connect(this->fd(), reinterpret_cast<sockaddr*>(&server), sizeof(server)) != 0)
                {
                    throw "Failed to bind socket";
                }
            }
        };

        template <typename Host>
        struct auto_closeable : public detail::socket_policy<Host>
        {
            ~auto_closeable()
            {
                this->self().close();
            }
        };
    }

    template <template <typename> typename ...Features>
    class Socket : public Features<Socket<Features>>...
    {
    public:
        Socket(int32_t descriptor)
            : descriptor_(descriptor)
        {}

        Socket()
            : descriptor_(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
        {
            fcntl(descriptor_, F_SETFL, O_NONBLOCK);
        }

        [[nodiscard]]
        auto read() -> std::optional<std::string>
        {
            constexpr auto chunk_size = 32ul;
            constexpr auto size_limit = 1024ul;

            auto message = std::string("");
            auto buffer = std::array<char, chunk_size>{};
            auto received = 0l;

            while ((received = ::read(descriptor_, buffer.data(), buffer.size())) > 0)
            {
                message.append(buffer.data(), received);
            }

            return received < 0
                ? std::nullopt
                : std::optional(message);
        }

        void write(std::string_view payload)
        {
            if (payload.size() != ::write(descriptor_, payload.data(), payload.size()))
            {
                throw "Failed to write to socket";
            }
        }

        void close()
        {
            if (descriptor_ != invalid_descriptor)
            {
                ::close(descriptor_);
            }
        }

        [[nodiscard]] constexpr
        auto descriptor() const -> int32_t
        { return descriptor_; }

    private:
        int32_t descriptor_ = invalid_descriptor;
    };

    using ServerSocket = Socket<socket::server, socket::auto_closeable>;
    using ClientSocket = Socket<socket::client, socket::auto_closeable>;
    using GenericSocket = Socket<socket::server, socket::client, socket::auto_closeable>;
    using ConnectionWrapper = Socket<socket::client>;

    class Epoll
    {
    public:
        Epoll() : descriptor_(epoll_create1(0)) {}

        void add(int32_t observed_fd) const
        {
            auto epoll_e = epoll_event{
                .events = EPOLLIN,
                .data = epoll_data{ .fd = observed_fd }
            };

            if (epoll_ctl(descriptor_, EPOLL_CTL_ADD, observed_fd, &epoll_e) != 0)
            {
                throw "Err: Epoll ctl failed";
            }
        }

        [[nodiscard]]
        auto wait()
        {
            constexpr auto event_to_descriptor = [](const auto& event)
                -> decltype(descriptor_) { return event.data.fd; };
            const auto count = std::max(0,
                epoll_wait(descriptor_, events_.data(), EPOLL_EVENT_MAX, EPOLL_TIMEOUT));

            return events_
                | std::views::take(count)
                | std::views::transform(event_to_descriptor);
        }

        [[nodiscard]] constexpr
        auto descriptor() const -> int32_t
        { return descriptor_; }

        ~Epoll()
        {
            if (descriptor_ != invalid_descriptor)
            {
                close(descriptor_);
            }
        }
        
    private:
        int32_t descriptor_ = -1;
        std::array<epoll_event, EPOLL_EVENT_MAX> events_{};
    };
}
