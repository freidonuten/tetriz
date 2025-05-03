#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <ranges>

#include "engine/bounding_box.hpp"
#include "engine/tetromino.hpp"


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

    constexpr auto is_occupied(const tetriz::Board& board, Coordinates coordinates)
    {
        return board[coordinates.y][coordinates.x] != Block::Void;
    }

    constexpr auto is_valid_coordinate(Coordinates coordinates)
    {
        const auto [x, y] = coordinates;

        return x >= 0
            && y >= 0
            && x < board_width
            && y < board_height;
    }

    constexpr auto blocks_of(tetriz::Tetromino tetromino)
    {
        return tetrominos[tetromino.shape][tetromino.rotation]
            | std::views::transform(std::bind_front(std::plus{}, tetromino.coordinates));
    }

    constexpr auto projects(const tetriz::Board& board, tetriz::Tetromino tetromino)
    {
        const auto blocks = blocks_of(tetromino);

        return std::ranges::all_of(blocks, is_valid_coordinate)
            && std::ranges::all_of(blocks, std::bind_front(std::not_fn(is_occupied), board));
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
}
