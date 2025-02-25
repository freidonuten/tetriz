#pragma once


#include "constants.h"
#include "crtp.hpp"

#include <format>
#include <ranges>
#include <cstdint>
#include <array>
#include <netinet/in.h>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <generator>


namespace net
{
    constexpr auto invalid_descriptor = -1;

    class socket_error : public std::runtime_error { using std::runtime_error::runtime_error; };
    class socket_bind_error : public socket_error { using socket_error::socket_error; };
    class socket_listen_error : public socket_error { using socket_error::socket_error; };
    class socket_connect_error : public socket_error { using socket_error::socket_error; };
    class socket_io_error : public socket_error { using socket_error::socket_error; };

    class epoll_error : public std::runtime_error { using std::runtime_error::runtime_error; };
    class epoll_create_error : public epoll_error { using epoll_error::epoll_error; };
    class epoll_add_error : public epoll_error { using epoll_error::epoll_error; };

    // Forward declaration needed for friending, see below
    namespace socket { template <typename Host> class auto_closeable; }

    template <template <typename> typename ...Features>
    class Socket : public Features<Socket<Features>>...
    {
        // since C++26 it will be possible to just befriend the whole pack like this:
        //     friend class Freatures<Socket<Features>>...
        // 
        // For now I'll just explicitly friend the feature which needs private access
        // which breaks the genericity but at least gets the job done without breaking
        // encapsulation.
        friend class socket::auto_closeable<Socket<Features...>>;

    public:
        Socket(int32_t descriptor)
            : descriptor_(descriptor)
        {
            fcntl(descriptor_, F_SETFL, O_NONBLOCK);
        }

        Socket()
            : Socket(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
        { }

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

            return message.empty()
                ? std::nullopt
                : std::optional(message);
        }

        void write(std::string_view payload)
        {
            if (payload.size() != ::write(descriptor_, payload.data(), payload.size()))
            {
                throw socket_io_error(std::format("Failed to write to socket {}", descriptor_));
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

        [[nodiscard]] constexpr
        auto is_valid() const -> bool
        { return descriptor_ != invalid_descriptor; }

        [[nodiscard]] constexpr
        explicit operator bool() const
        { return is_valid(); }

    private:
        int32_t descriptor_ = invalid_descriptor;
    };

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
        struct client : public detail::socket_policy<Host>
        {
            void connect(std::string_view address, uint16_t port) const
            {
                auto host = in_addr_t{ inet_addr(address.data()) };
                auto server = sockaddr_in{ AF_INET, htons(port), host };

                if (::connect(this->fd(), std::bit_cast<sockaddr*>(&server), sizeof(server)) != 0)
                {
                    throw socket_connect_error(std::format("Failed to connect to {}:{}", address, port));
                }
            }
        };


        template <typename Host>
        struct server : public detail::socket_policy<Host>
        {
            void bind(std::string_view address, uint16_t port)
            {
                auto host = in_addr_t{ inet_addr(address.data()) };
                auto server = sockaddr_in{ AF_INET, htons(port), host };

                if (const auto err = ::bind(this->fd(), reinterpret_cast<sockaddr*>(&server), sizeof(server)); err != 0)
                {
                    throw socket_bind_error(std::format("failed to bind to {}:{}: {}", address, port, strerror(err)));
                }
            }

            void listen()
            {
                static constexpr auto queue_size = 100;

                if (::listen(this->fd(), queue_size) == -1)
                {
                    throw socket_listen_error(std::format("listend failed on descriptor {}", this->fd()));
                }
            }

            [[nodiscard]]
            auto accept() -> std::generator<uint32_t>
            {
                struct sockaddr_in client{};
                socklen_t client_len = sizeof(struct sockaddr_in);

                while (true)
                {
                    auto fd = ::accept(this->fd(), std::bit_cast<struct sockaddr *>(&client), &client_len);
                    if (fd == invalid_descriptor)
                        break;

                    co_yield fd;
                }
            }
        };

        template <typename Host>
        struct auto_closeable : public detail::socket_policy<Host>
        {
            auto_closeable() = default;
            auto_closeable(const auto_closeable&) = delete;
            auto_closeable(auto_closeable&& other) noexcept
            {
                this->self().descriptor_ = other.self().descriptor_;
                other.self().descriptor_ = invalid_descriptor;
            }

            auto operator=(const auto_closeable&) -> auto_closeable& = delete;
            auto operator=(auto_closeable&& other) noexcept -> auto_closeable&
            {
                new (this) Host(other);
                return *this;
            }

            ~auto_closeable() { this->self().close(); }
        };
    }

    using ServerSocket = Socket<socket::server, socket::auto_closeable>;
    using ClientSocket = Socket<socket::client, socket::auto_closeable>;
    using GenericSocket = Socket<socket::server, socket::client, socket::auto_closeable>;
    using ConnectionWrapper = Socket<socket::client>;

    class Epoll
    {
    public:
        Epoll()
            : descriptor_(epoll_create1(0))
        {
            if (descriptor_ == invalid_descriptor)
            {
                throw epoll_create_error("Failed to create epoll instance");
            }
        }

        void add(int32_t observed_fd) const
        {
            auto epoll_e = epoll_event{
                .events = EPOLLIN,
                .data = epoll_data{ .fd = observed_fd }
            };

            if (epoll_ctl(descriptor_, EPOLL_CTL_ADD, observed_fd, &epoll_e) != 0)
            {
                throw epoll_add_error(std::format(
                    "Failed to add descriptor {} to epoll instance {}", observed_fd, descriptor_));
            }
        }

        [[nodiscard]]
        auto wait()
        {
            constexpr auto event_to_descriptor = [](const auto& event)
                -> Socket<socket::client> { return event.data.fd; };
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
