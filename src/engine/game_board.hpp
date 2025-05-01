#pragma once

#include <array>
#include <cstdint>
#include <ranges>

#include "engine/bounding_box.hpp"
#include "engine/offsets.hpp"
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

    constexpr auto is_occupied(Block block) -> bool
    {
        return block != Block::Void;
    }

    using Board = std::array<std::array<Block, board_width>, board_height>;

    constexpr auto projects(const tetriz::Board& board, tetriz::Tetromino tetromino)
    {
        const auto bb_size = bounding_box_sizes[tetromino.shape];
        const auto offsets = tetriz::offsets[tetromino.shape][tetromino.rotation];
        const auto [curr_x, curr_y] = tetromino.coordinates;

        if (curr_x + offsets[Side::Left] < 0
         || curr_x - offsets[Side::Right] + bb_size > board_width
         || curr_y - offsets[Side::Bottom] + bb_size > board_height)
            return false;

        const auto bounding_box = bounding_boxes[tetromino.shape][tetromino.rotation];
        for (auto y = offsets[Side::Top]; y < bb_size - offsets[Side::Bottom]; ++y)
            for (auto x = offsets[Side::Left]; x < bb_size - offsets[Side::Right]; ++x)
                if (bounding_box[y][x] && is_occupied(board[y + curr_y][x + curr_x]))
                    return false;

        return true;
    }

    constexpr void project(tetriz::Board& board, tetriz::Tetromino tetromino, Block block)
    {
        const auto [base_x, base_y] = tetromino.coordinates;
        const auto bounding_box = tetriz::bounding_boxes[tetromino.shape][tetromino.rotation];

        for (const auto& [y, row] : bounding_box | std::views::enumerate)
            for (const auto [x, cell] : row | std::views::enumerate)
            {
                if ((base_x + x) >= board_width || (base_y + y) >= board_height)
                    continue;

                if (cell)
                    board[base_y + y][base_x + x] = block;
            }
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
