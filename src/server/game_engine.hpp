#pragma once

#include "engine/game.hpp"
#include "proto/protocol.hpp"
#include "networking_socket.hpp"


using namespace std::chrono_literals;


class GameEngine
{
public:
    void action(tetriz::proto::Move move)
    {
        using tetriz::proto::Move;
        using tetriz::Direction;

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
    }

    void tick()
    {
        game_.tick();
    }

    auto game() const
        -> const tetriz::Game&
    { return game_; }

private:
    tetriz::Game game_;
};
