#pragma once

#include <chrono>
#include <cstdint>
#include <variant>

#include "engine/game.hpp"
#include "engine/game_board.hpp"

#include "util/time.hpp"

#include "serializers.hpp"


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
        uint8_t player_id{};
        Board board{};
        Tetromino current{};
        std::optional<TetrominoShape> swap = {};
        Bag bag{TetrominoShape::T, TetrominoShape::T, TetrominoShape::T, TetrominoShape::T};
        uint16_t score{};
    };

    struct DatagramTime
    {
        Duration timestamp;
    };

    struct DatagramHola
    {
        uint8_t room_size;
    };

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
                        .player_id = pop_from<uint8_t>(message),
                        .board = pop_from<Board>(message),
                        .current = pop_from<Tetromino>(message),
                        .swap = pop_from<std::optional<TetrominoShape>>(message),
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
                    .payload = DatagramHola{
                        .room_size = pop_from<uint8_t>(message)
                    }
                };
            default:
                return std::nullopt;
        }
    }

    constexpr auto serialize_move(Move move)
    {
        return serialize(MessageType::Move, move);
    }

    constexpr auto serialize_game(uint8_t player_id, const Game& game)
    {
        return serialize(
            MessageType::Game,
            player_id,
            game.board(),
            game.current(),
            game.swapped(),
            game.bag().peek<sizeof(DatagramGame::bag)>(),
            game.score()
        );
    }

    constexpr auto serialize_time(Duration timestamp)
    {
        return serialize(MessageType::Time, timestamp);
    }

    constexpr auto serialize_hola(uint8_t room_size)
    {
        return serialize(MessageType::Hola, room_size);
    }
}
