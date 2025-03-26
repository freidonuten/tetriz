#include <csignal>
#include <map>
#include <thread>

#include "engine/game.hpp"
#include "epoll.hpp"
#include "logger.hpp"
#include "proto/protocol.hpp"

using namespace std::chrono_literals;

bool run = true;

void signal_handler(int signal)
{
    run = false;
}


class GameEngine
{
public:
    GameEngine(net::ConnectionWrapper client)
        : client_(client)
        , worker_(std::bind_front(&GameEngine::start, this))
    {}

    ~GameEngine()
    {
        run_ = false;
    }

    void action(tetriz::proto::Move move)
    {
        using tetriz::proto::Move;
        using tetriz::Direction;

        log_info("Received action: {}", magic_enum::enum_name(move));

        [&] {
            switch (move)
            {
                case Move::Left:   return game_.move(Direction::Left);
                case Move::Right:  return game_.move(Direction::Right);
                case Move::Down:   return game_.move(Direction::Down);
                case Move::Drop:   return game_.drop();
                case Move::Rotate: return game_.rotate();
                case Move::Swap:   return game_.swap();
            }
        }();

        client_.write(tetriz::proto::serialize_sync(0s, game_));
    }

private:
    void start()
    {
        using tetriz::proto::serialize_sync;

        auto begin_time = std::chrono::high_resolution_clock::now();
        auto tick = std::chrono::duration<uint32_t, std::milli>{1'000};

        while (run_)
        {
            std::this_thread::sleep_until(begin_time + tick);
            game_.tick();
            client_.write(serialize_sync(tick, game_));
            tick += 1s;
        }
    }

    std::atomic<bool> run_ = true;
    tetriz::Game game_;
    net::ConnectionWrapper client_;
    std::jthread worker_;
};

std::map<int32_t, GameEngine> games;


auto hexdump_byte(uint8_t byte) -> std::string
{
    static constexpr auto digits = std::to_array({
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'A', 'B',
        'C', 'D', 'E', 'F'
    });

    return { digits[(byte & 0xF0) >> 4], digits[byte & 0x0F] };
}

auto hexdump(std::span<const uint8_t> range) -> std::string
{
    return range
        | std::views::transform(hexdump_byte)
        | std::views::join
        | std::ranges::to<std::string>();
}

void notify(net::ConnectionWrapper client)
{
    if (const auto payload = client.read(); payload)
    {
        if (!games.contains(client.descriptor()))
        {
            log_info("Opening...");
            games.emplace(std::piecewise_construct, std::forward_as_tuple(client.descriptor()), std::forward_as_tuple(client));
        }

        log_info("Received: {}", hexdump(*payload));
        auto msg = tetriz::proto::deserialize(*payload);
        if (msg.has_value() && msg->type == tetriz::proto::MessageType::Move)
        {
            games.at(client.descriptor()).action(std::get<tetriz::proto::DatagramMove>(msg->payload).move);
        }
        else
        {
            log_info("Received unexpected message type!");
        }
    }
    else
    {
            log_info("Closing...");
            games.erase(client.descriptor());
            client.close();
    }
}

auto main(int argc, char** argv) -> int
{
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    auto epoll = make_epoll();
    auto socket = net::ServerSocket();
    socket.set_non_blocking();
    socket.bind("127.0.0.1", 6666);
    socket.listen();

    log_info("Starting event loop");

    while (run)
    {
        for (auto client : socket.accept())
        {
            net::Socket(client).set_nodelay();
            epoll->add(client);
        }

        for (const auto client : epoll->wait())
            notify(client);
    }
}
