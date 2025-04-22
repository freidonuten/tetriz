#pragma once


#include <format>
#include <cstdint>
#include <array>
#include <vector>
#include <netinet/in.h>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
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


    // Forward declaration needed for friending, see below
    namespace socket { template <typename> class auto_closeable; }

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
        { }

        Socket()
            : Socket(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
        { }

        auto set_non_blocking()
        {
            return fcntl(descriptor_, F_SETFL, O_NONBLOCK) == 0;
        }

        auto set_nodelay(bool value = true)
        {
            constexpr auto val = 1;
            return setsockopt(descriptor_, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == 0;
        }

        [[nodiscard]]
        auto read() -> std::optional<std::vector<uint8_t>>
        {
            constexpr auto chunk_size = 32ul;

            auto message = std::vector<uint8_t>{};
            auto buffer = std::array<char, chunk_size>{};
            auto received = 0l;

            while ((received = ::recv(descriptor_, buffer.data(), buffer.size(), 0)) > 0)
                std::copy_n(buffer.begin(), received, std::back_inserter(message));

            return message.empty()
                ? std::nullopt
                : std::optional(message);
        }

        void write(std::span<const uint8_t> payload) const
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

        auto operator<=>(const Socket& other) const
        { return descriptor_ <=> other.descriptor_; }

    private:
        int32_t descriptor_ = invalid_descriptor;
    };

    namespace socket
    {
        namespace detail
        {
            template <typename Host>
            struct socket_policy
            {
                auto self(this auto& self) -> Host&
                {
                    return static_cast<Host&>(self);
                }

                auto self(this const auto& self) -> const Host&
                {
                    return static_cast<const Host&>(self);
                }
            };
        }

        template <typename Host>
        struct client : protected detail::socket_policy<Host>
        {
            void connect(std::string_view address, uint16_t port) const
            {
                auto host = in_addr_t{ inet_addr(address.data()) };
                auto server = sockaddr_in{ AF_INET, htons(port), host };

                if (::connect(this->self().descriptor(), std::bit_cast<sockaddr*>(&server), sizeof(server)) != 0)
                {
                    throw socket_connect_error(std::format("Failed to connect to {}:{} ({})", address, port, strerror(errno)));
                }
            }
        };


        template <typename Host>
        struct server : protected detail::socket_policy<Host>
        {
            void bind(std::string_view address, uint16_t port)
            {
                auto host = in_addr_t{ inet_addr(address.data()) };
                auto server = sockaddr_in{ AF_INET, htons(port), host };

                if (const auto err = ::bind(this->self().descriptor(), reinterpret_cast<sockaddr*>(&server), sizeof(server)); err != 0)
                {
                    throw socket_bind_error(std::format("failed to bind to {}:{}: {}", address, port, strerror(err)));
                }
            }

            void listen()
            {
                static constexpr auto queue_size = 100;

                if (::listen(this->self().descriptor(), queue_size) == -1)
                {
                    throw socket_listen_error(std::format("listend failed on descriptor {}", this->self().descriptor()));
                }
            }

            [[nodiscard]]
            auto accept() -> std::generator<uint32_t>
            {
                struct sockaddr_in client{};
                socklen_t client_len = sizeof(struct sockaddr_in);

                while (true)
                {
                    auto descriptor = ::accept(this->self().descriptor(), std::bit_cast<struct sockaddr *>(&client), &client_len);
                    if (descriptor == invalid_descriptor)
                        break;

                    co_yield descriptor;
                }
            }
        };

        template <typename Host>
        struct auto_closeable : protected detail::socket_policy<Host>
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

            ~auto_closeable()
            {
                this->self().close();
            }
        };
    }

    using ServerSocket = Socket<socket::server, socket::auto_closeable>;
    using ClientSocket = Socket<socket::client, socket::auto_closeable>;
    using GenericSocket = Socket<socket::server, socket::client, socket::auto_closeable>;
    using ConnectionWrapper = Socket<socket::client>;

}
