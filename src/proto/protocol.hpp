#pragma once

#include <chrono>
#include <cstdint>
#include <variant>

#include "engine/game.hpp"
#include "engine/game_board.hpp"

#include "proto/serializers.hpp"
#include "util/time.hpp"


namespace tetriz::proto
{
    enum class MessageType : uint8_t
    {
        Move,
        Game,
        Time,
        Hola,
    };

    enum class Move : uint8_t
    {
        Left,
        Right,
        Down,
        Drop,
        Rotate,
        Swap
    };

    struct DatagramMove
    {
        Move move{};
    };

    using Bag = std::array<TetrominoShape, 4>;

    struct DatagramGame
    {
        Board board{};
        Tetromino current{};
        TetrominoShape swap = TetrominoShape::T;
        Bag bag{TetrominoShape::T, TetrominoShape::T, TetrominoShape::T, TetrominoShape::T};
        uint16_t score{};
    };

    struct DatagramTime
    {
        Duration timestamp;
    };

    struct DatagramHola {};

    using Payload = std::variant<
        std::monostate,
        DatagramMove,
        DatagramGame,
        DatagramTime,
        DatagramHola
    >;

    struct Datagram
    {
        MessageType type{};
        Payload payload{};
    };

    constexpr auto deserialize(std::span<const uint8_t> message) -> std::optional<Datagram>
    {
        switch (pop_from<MessageType>(message))
        {
            case MessageType::Move:
                return Datagram{
                    .type = MessageType::Move,
                    .payload = pop_from<DatagramMove>(message)
                };
            case MessageType::Game:
                return Datagram{
                    .type = MessageType::Game,
                    .payload = DatagramGame{
                        .board = pop_from<Board>(message),
                        .current = pop_from<Tetromino>(message),
                        .swap = pop_from<TetrominoShape>(message),
                        .bag = pop_from<Bag>(message),
                        .score = pop_from<uint16_t>(message)
                    }
                };
            case MessageType::Time:
                return Datagram{
                    .type = MessageType::Time,
                    .payload = DatagramTime{
                        .timestamp = pop_from<Duration>(message)
                    }
                };
            case MessageType::Hola:
                return Datagram{
                    .type = MessageType::Hola,
                    .payload = DatagramHola{}
                };
            default:
                return std::nullopt;
        }
    }

    constexpr auto serialize_move(Move move)
    {
        return serialize(MessageType::Move, move);
    }

    constexpr auto serialize_game(const Game& game)
    {
        return serialize(
            MessageType::Game,
            game.board(),
            game.current(),
            game.swapped().value_or(TetrominoShape::I),
            game.bag().peek<sizeof(DatagramGame::bag)>(),
            game.score()
        );
    }

    constexpr auto serialize_time(Duration timestamp)
    {
        return serialize(MessageType::Time, timestamp);
    }

    constexpr auto serialize_hola()
    {
        return serialize(MessageType::Hola);
    }
}
