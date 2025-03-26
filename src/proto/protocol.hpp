#pragma once

#include <chrono>
#include <cstdint>
#include <variant>

#include "engine/game.hpp"
#include "engine/game_board.hpp"

#include "logger.hpp"
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

    constexpr auto deserialize(std::vector<uint8_t> payload)
        -> std::optional<Datagram>
    {
        switch (MessageType(payload[0]))
        {
            case MessageType::Move:
                return Datagram{
                    .type = MessageType::Move,
                    .payload = DatagramMove{
                        .move = Move(payload[1])
                    }
                };
            case MessageType::Sync:
                return Datagram{
                    .type = MessageType::Sync,
                    .payload = DatagramSync{
                        .board = *reinterpret_cast<Board*>(payload.data() + 1),
                        .current = {
                            .shape = TetrominoShape(payload[sizeof(Board) + 1]),
                            .rotation = TetrominoRotation(payload[sizeof(Board) + 2]),
                            .coordinates = {
                                .x = static_cast<int8_t>(payload[sizeof(Board) + 3]),
                                .y = static_cast<int8_t>(payload[sizeof(Board) + 4]),
                            }},
                        .swap = TetrominoShape(payload[sizeof(Board) + 5]),
                        .bag = {
                            TetrominoShape(payload[sizeof(Board) + 6]),
                            TetrominoShape(payload[sizeof(Board) + 7]),
                            TetrominoShape(payload[sizeof(Board) + 8]),
                            TetrominoShape(payload[sizeof(Board) + 9]),
                        },
                        .timestamp = *reinterpret_cast<decltype(DatagramSync::timestamp)*>(&payload[sizeof(Board) + 10]),
                        .score = *reinterpret_cast<uint16_t*>(&payload[sizeof(Board) + 14])
                    }
                };
        }
    }

    constexpr auto serialize_move(Move move)
        -> std::vector<uint8_t>
    {
        return {
            static_cast<uint8_t>(MessageType::Move),
            static_cast<uint8_t>(move)
        };
    }

    constexpr auto serialize_sync(decltype(DatagramSync::timestamp) timestamp, const Game& game)
        -> std::vector<uint8_t>
    {
        auto result = std::vector<uint8_t>{
            static_cast<uint8_t>(MessageType::Sync)
        };

        const auto board_raw = game.board()
            | std::views::join
            | std::views::transform(fn_scast<uint8_t>);

        std::ranges::copy(board_raw, std::back_inserter(result));

        result.push_back(static_cast<uint8_t>(game.current().shape));
        result.push_back(static_cast<uint8_t>(game.current().rotation));
        result.push_back(static_cast<uint8_t>(game.current().coordinates.x));
        result.push_back(static_cast<uint8_t>(game.current().coordinates.y));
        result.push_back(static_cast<uint8_t>(game.swapped().value_or(TetrominoShape::I)));

        const auto bag_raw = game.bag().peek<sizeof(DatagramSync::bag)>()
            | std::views::transform(fn_scast<uint8_t>);

        std::ranges::copy(bag_raw, std::back_inserter(result));
        std::ranges::copy(std::bit_cast<std::array<uint8_t, 4>>(timestamp.count()), std::back_inserter(result));
        std::ranges::copy(std::bit_cast<std::array<uint8_t, 2>>(game.score()), std::back_inserter(result));

        log_info("payload = {}B", result.size());

        return result;
    }
}
