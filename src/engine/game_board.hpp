#pragma once

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
    };

    constexpr auto is_occupied(Block block) -> bool
    {
        return block != Block::Void;
    }

    using Board = std::array<std::array<Block, board_width>, board_height>;

    constexpr void project_on_board(tetriz::Board& board, tetriz::Tetromino tetromino)
    {
        const auto [curr_x, curr_y] = tetromino.coordinates;
        const auto bounding_box = tetriz::bounding_boxes[tetromino.shape][tetromino.rotation];

        for (const auto& [y, row] : bounding_box | std::views::enumerate)
            for (const auto [x, block] : row | std::views::enumerate)
                if (block)
                    board[curr_y + y][curr_x + x] = static_cast<Block>(tetromino.shape);
    }

    constexpr auto project_on_board(const tetriz::Board& board, tetriz::Tetromino tetromino) -> Board
    {
        auto board_new = board;
        project_on_board(board_new, tetromino);
        return board_new;
    }
}
