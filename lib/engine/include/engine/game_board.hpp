#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <ranges>

#include "bounding_box.hpp"
#include "engine/kick_table.hpp"
#include "tetromino.hpp"


namespace tetriz
{
    static constexpr auto board_width = 10u;
    static constexpr auto board_height = 22u;

    enum class Block : uint8_t
    {
        Void,
        Cyan = static_cast<uint8_t>(TetrominoShape::I),
        Blue = static_cast<uint8_t>(TetrominoShape::J),
        Orange = static_cast<uint8_t>(TetrominoShape::L),
        Yellow = static_cast<uint8_t>(TetrominoShape::O),
        Lime = static_cast<uint8_t>(TetrominoShape::S),
        Magenta = static_cast<uint8_t>(TetrominoShape::T),
        Red = static_cast<uint8_t>(TetrominoShape::Z),
        Shadow
    };

    using Board = std::array<std::array<Block, board_width>, board_height>;

    constexpr auto is_occupied(const tetriz::Board& board, Coordinates coordinates) -> bool
    {
        return board[coordinates.y][coordinates.x] != Block::Void;
    }

    constexpr auto is_empty(const tetriz::Board& board, Coordinates coordinates) -> bool
    {
        return board[coordinates.y][coordinates.x] == Block::Void;
    }

    constexpr auto is_valid_coordinate(Coordinates coordinates)
    {
        const auto [x, y] = coordinates;

        return x >= 0
            && y >= 0
            && static_cast<uint32_t>(x) < board_width
            && static_cast<uint32_t>(y) < board_height;
    }

    constexpr auto blocks_of(tetriz::Tetromino tetromino)
    {
        return tetrominos[tetromino.shape][tetromino.rotation]
            | std::views::transform(std::bind_front(std::plus{}, tetromino.coordinates));
    }

    constexpr auto projects(const tetriz::Board& board, tetriz::Tetromino tetromino) -> bool
    {
        const auto blocks = blocks_of(tetromino);

        return std::ranges::all_of(blocks, is_valid_coordinate)
            && std::ranges::all_of(blocks, std::bind_front(is_empty, board));
    }

    constexpr void project(tetriz::Board& board, tetriz::Tetromino tetromino, Block block)
    {
        for (const auto [x, y] : blocks_of(tetromino))
            board[y][x] = block;
    }

    constexpr void project(tetriz::Board& board, tetriz::Tetromino tetromino)
    {
        project(board, tetromino, static_cast<Block>(tetromino.shape));
    }

    constexpr auto project(const tetriz::Board& board, tetriz::Tetromino tetromino) -> Board
    {
        auto board_new = board;
        project(board_new, tetromino);
        return board_new;
    }

    constexpr auto project_with_shadow(const tetriz::Board& board, tetriz::Tetromino tetromino) -> Board
    {
        auto board_new = board;
        const auto original_tetromino = tetromino;

        while (projects(board, tetromino))
            ++tetromino.coordinates.y;

        --tetromino.coordinates.y;

        // you need to project the shadow first so that it does not overlap the actual shape
        project(board_new, tetromino, Block::Shadow);
        project(board_new, original_tetromino);

        return board_new;
    }

    constexpr auto rotate(const Board& board, Tetromino tetromino) -> std::optional<Tetromino>
    {
        const auto rotation = next_left(tetromino.rotation);

        for (const auto offset : kick_offsets(tetromino.shape, rotation))
        {
            const auto candidate = Tetromino{
                .shape = tetromino.shape,
                .rotation = rotation,
                .coordinates = tetromino.coordinates + offset
            };

            if (projects(board, candidate))
                return candidate;
        }

        return tetromino;
    }

    constexpr auto drop(const Board& board, Tetromino tetromino)
    {
        constexpr auto move_down = Coordinates{ 0, 1 };

        while (projects(board, tetromino + move_down))
            tetromino += move_down;

        return tetromino;
    }

    constexpr auto clear(Board& board) -> size_t
    {
        auto count = 0z;

        for (auto& row : board | std::views::drop(2))
            if (count++; !std::ranges::contains(row, Block::Void))
                std::copy_backward(board.begin(), &row, &row);

        return count;
    }

    constexpr auto spawn(const Board& board, TetrominoShape tetromino) -> std::optional<Tetromino>
    {
        auto spawned = Tetromino{
            .shape = tetromino,
            .rotation = TetrominoRotation::Base,
            .coordinates = { 3, 1 }
        };

        for (const auto offset : {Coordinates{0, 0}, Coordinates{0, -1}})
        {
            if (is_empty(board, offset))
            {
                spawned.coordinates += offset;
                return spawned;
            }
        }

        return std::nullopt;
    }
}
