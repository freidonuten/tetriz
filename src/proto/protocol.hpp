#pragma once

#include <chrono>
#include <cstdint>
#include <variant>

#include "engine/game.hpp"
#include "engine/game_board.hpp"

#include "logger.hpp"
#include "proto/serializers.hpp"
#include "util/functional.hpp"


namespace tetriz::proto
{
    enum class MessageType : uint8_t
    {
        Move,
        Sync,
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

    struct DatagramSync
    {
        Board board{};
        Tetromino current{};
        TetrominoShape swap = TetrominoShape::T;
        std::array<TetrominoShape, 4> bag{
            TetrominoShape::T, TetrominoShape::T, TetrominoShape::T, TetrominoShape::T
        };
        std::chrono::duration<uint32_t, std::milli> timestamp{};
        uint16_t score{};
    };

    using Payload = std::variant<std::monostate, DatagramMove, DatagramSync>;

    struct Datagram
    {
        MessageType type{};
        Payload payload{};
    };

    constexpr auto deserialize(const std::vector<uint8_t>& vec)
        -> std::optional<Datagram>
    {
        auto payload = std::span<const uint8_t>(vec);
        switch (pop_from<MessageType>(payload))
        {
            case MessageType::Move:
                return Datagram{
                    .type = MessageType::Move,
                    .payload = pop_from<DatagramMove>(payload)
                };
            case MessageType::Sync:
                return Datagram{
                    .type = MessageType::Sync,
                    .payload = DatagramSync{
                        .board = pop_from<Board>(payload),
                        .current = pop_from<Tetromino>(payload),
                        .swap = pop_from<TetrominoShape>(payload),
                        .bag = pop_from<std::array<TetrominoShape, 4>>(payload),
                        .timestamp = pop_from<const decltype(DatagramSync::timestamp)>(payload),
                        .score = pop_from<uint16_t>(payload)
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

    constexpr auto serialize_sync(decltype(DatagramSync::timestamp) timestamp, const Game& game)
    {
        return serialize(
            MessageType::Sync,
            game.board(),
            game.current(),
            game.swapped().value_or(TetrominoShape::I),
            game.bag().peek<sizeof(DatagramSync::bag)>(),
            timestamp.count(),
            game.score()
        );
    }
}
