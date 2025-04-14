#pragma once

#include "engine/game.hpp"
#include "proto/protocol.hpp"
#include "networking_socket.hpp"


using namespace std::chrono_literals;


class GameEngine
{
public:
    GameEngine(net::ConnectionWrapper client)
        : client_(client)
    {}

    GameEngine(const GameEngine&) = default;
    GameEngine(GameEngine&& other) = default;

    void action(tetriz::proto::Move move)
    {
        using tetriz::proto::Move;
        using tetriz::Direction;

        log_info("Received action: {}", magic_enum::enum_name(move));

        [this, move] {
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

        update_client();
    }

    void tick()
    {
        game_.tick();
        update_client();
    }

private:
    tetriz::Game game_;
    net::ConnectionWrapper client_;

    void update_client()
    {
        client_.write(tetriz::proto::serialize_game(game_));
    }
};
