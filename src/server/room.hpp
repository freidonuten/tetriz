#pragma once

#include <map>
#include <thread>

#include "server/game_engine.hpp"
#include "util/encoding.hpp"
#include "util/time.hpp"
#include "proto/protocol.hpp"


class Room
{
public:
    Room() = default;

    ~Room()
    {
        worker_.join();
    }

    void add_player(net::ConnectionWrapper player)
    {
        games_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(player),
                std::forward_as_tuple(player));

        if (games_.size() == 2)
            start();
    }

    void notify(net::ConnectionWrapper player, std::span<const uint8_t> message)
    {
        log_info("Received: {}", hexdump(message));

        if (start_time_ > Clock::now())
        {
            log_trace("Won't notify, game has not started yet");
            return;
        }


        const auto msg = tetriz::proto::deserialize(message);

        if (!msg)
        {
            log_warning("Invalid message!");
            return;
        }

        if (msg->type == tetriz::proto::MessageType::Move)
        {
            games_
                .at(player.descriptor())
                .action(std::get<tetriz::proto::DatagramMove>(msg->payload).move);

            return;
        }

        log_info("Received unexpected message type!");
    }

private:
    std::map<net::ConnectionWrapper, GameEngine> games_;
    std::jthread worker_ = {};
    std::atomic<bool> run_ = true;
    TimePoint start_time_ = TimePoint::max();

    void start()
    {
        static constexpr auto countdown_length = 5s;

        if (worker_.joinable())
            return;

        start_time_ = Clock::now() + countdown_length;
        worker_ = std::jthread([this]{
            for (auto tick = -countdown_length; run_; tick += 1s)
            {
                std::this_thread::sleep_until(start_time_ + tick);

                if (start_time_ < Clock::now())
                    std::ranges::for_each(games_ | std::views::values, &GameEngine::tick);

                std::ranges::for_each(games_ | std::views::keys, [this](const auto& client){
                    client.write(
                        tetriz::proto::serialize_time(
                            std::chrono::duration_cast<Duration>(Clock::now() - start_time_)));
                });
            }
        });
    }
};
