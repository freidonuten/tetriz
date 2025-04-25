#pragma once

#include <map>
#include <thread>

#include "networking_socket.hpp"
#include "server/game_engine.hpp"
#include "util/time.hpp"
#include "proto/protocol.hpp"


class Room
{
public:
    Room(uint32_t room_size)
        : room_size_(room_size)
    {}

    void notify(net::ConnectionWrapper client, const tetriz::proto::Datagram& message)
    {
        if (!games_.contains(client))
        {
            if (message.type == tetriz::proto::MessageType::Hola)
            {
                log_debug("Player joined");
                add_player(client);
            }
            else
            {
                log_debug("Expected Hola");
                client.close();
            }

            return;
        }

        if (message.type == tetriz::proto::MessageType::Move)
        {
            if (start_time_ > Clock::now())
            {
                log_trace("Won't notify, game has not started yet");
                return;
            }

            games_
                .at(client.descriptor())
                .action(std::get<tetriz::proto::DatagramMove>(message.payload).move);

            //notify_move(client);
            notify_tick();
            return;
        }

        log_info("Received unexpected message type!");
    }

    void leave(net::ConnectionWrapper client)
    {
        games_.erase(client);
        client.close();
    }

    void stop()
    {
        run_.exchange(false);
    }

    auto has_member(net::ConnectionWrapper client) const -> bool
    {
        return games_.contains(client);
    }

    auto empty() const -> bool
    {
        return games_.empty();
    }

    auto has_slot() const -> bool
    {
        return games_.size() < room_size_;
    }

    auto size() const -> size_t
    {
        return room_size_;
    }

private:
    inline static auto room_id = 0u;

    uint32_t room_id_ = ++room_id;
    uint32_t room_size_ = 0;
    uint32_t room_seed_ = Clock::now().time_since_epoch().count();
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

                log_trace("room #{}: tick", room_id_);

                if (start_time_ < Clock::now())
                    std::ranges::for_each(games_ | std::views::values, &GameEngine::tick);

                std::ranges::for_each(games_ | std::views::keys, [this](const auto& client){
                    client.write(
                        tetriz::proto::serialize_time(
                            std::chrono::duration_cast<Duration>(Clock::now() - start_time_)));
                });

                notify_tick();
            }
        });
    }

    void add_player(net::ConnectionWrapper player)
    {
        games_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(player),
                std::forward_as_tuple(room_seed_));

        if (games_.size() == room_size_)
            start();
    }

    // FIXME Broken indexing
    void notify_move(net::ConnectionWrapper originator_sock)
    {
        const auto& originator_game = games_.at(originator_sock).game();

        auto id = uint16_t{0};
        for (const auto& another_sock : games_ | std::views::keys)
        {
            if (originator_sock.descriptor() == another_sock.descriptor())
                another_sock.write(tetriz::proto::serialize_game(0, originator_game));
            else
                another_sock.write(tetriz::proto::serialize_game(++id, originator_game));
        }
    }

    void notify_tick()
    {
        for (const auto& current_sock : games_ | std::views::keys)
        {
            auto id = uint16_t{0};
            for (const auto& [another_sock, engine] : games_)
            {
                if (current_sock.descriptor() == another_sock.descriptor())
                    current_sock.write(tetriz::proto::serialize_game(0, engine.game()));
                else
                    current_sock.write(tetriz::proto::serialize_game(++id, engine.game()));
            }
        }
    }
};
