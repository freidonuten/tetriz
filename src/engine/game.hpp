#pragma once

#include "engine/bounding_box.hpp"
#include "engine/game_board.hpp"
#include "engine/kick_table.hpp"
#include "engine/offsets.hpp"
#include "engine/tetromino.hpp"
#include "engine/tetromino_bag.hpp"


namespace tetriz
{
    enum class Direction : uint8_t
    {
        Left,
        Right,
        Down
    };

    constexpr inline auto x_shift = magic_enum::containers::array<Direction, int8_t>{ -1, 1, 0 };
    constexpr inline auto y_shift = magic_enum::containers::array<Direction, int8_t>{ 0, 0, 1 };

    enum class State : uint8_t
    {
        Playing,
        Finished,
        GameOver
    };

    class Game
    {
    public:
        constexpr Game(uint32_t seed)
            : bag_(seed)
        {
            spawn(bag_.poll());
        }

        constexpr void move(Direction direction)
        {
            const auto x_shift = tetriz::x_shift[direction];
            const auto y_shift = tetriz::y_shift[direction];
            
            if (is_empty(x_shift, y_shift))
            {
                current_.coordinates.x += x_shift;
                current_.coordinates.y += y_shift;
            }
            else if (direction == Direction::Down)
            {
                lock();
            }
        }

        constexpr void tick()
        {
            move(Direction::Down);
        }

        constexpr void drop()
        {
            while (is_empty(0, 1))
                current_.coordinates.y += 1;

            lock();
        }

        constexpr void rotate()
        {
            current_.rotation = next_left(current_.rotation);

            for (auto [x_offset, y_offset] : kick_offsets(current_.shape, current_.rotation))
            {
                if (is_empty(x_offset, y_offset))
                {
                    current_.coordinates.x += x_offset;
                    current_.coordinates.y += y_offset;
                    return;
                }
            }

            current_.rotation = next_right(current_.rotation);
        }

        constexpr void swap()
        {
            if (just_swapped_)
                return;

            if (!swapped_)
                swapped_ = bag_.poll();

            const auto original_shape = current_.shape;
            spawn(*swapped_);
            swapped_ = original_shape;
            just_swapped_ = true;
        }

        constexpr auto state() const -> State { return state_; }
        constexpr auto board() const -> const Board& { return board_; }
        constexpr auto current() const -> const Tetromino& { return current_; }
        constexpr auto score() const -> uint16_t { return score_; }
        constexpr auto bag() const -> const TetrominoBag& { return bag_; }
        constexpr auto swapped() const -> const std::optional<TetrominoShape>& { return swapped_; }

    private:
        constexpr auto is_empty(int x_offset, int y_offset) const -> bool
        {
            const auto bb_size = bounding_box_sizes[current_.shape];
            const auto offsets = tetriz::offsets[current_.shape][current_.rotation];
            const auto [curr_x, curr_y] = current_.coordinates;

            if (curr_x + x_offset + offsets[Side::Left] < 0
             || curr_x + x_offset - offsets[Side::Right] + bb_size > board_width
             || curr_y + y_offset - offsets[Side::Bottom] + bb_size > board_height)
                return false;

            const auto bounding_box = bounding_boxes[current_.shape][current_.rotation];
            for (auto y = offsets[Side::Top]; y < bb_size - offsets[Side::Bottom]; ++y)
                for (auto x = offsets[Side::Left]; x < bb_size - offsets[Side::Right]; ++x)
                    if (bounding_box[y][x] && is_occupied(board_[y + curr_y + y_offset][x + curr_x + x_offset]))
                        return false;

            return true;
        }

        constexpr void try_lock()
        {
            if (!is_empty(0, 1))
                lock();
        }

        constexpr void lock()
        {
            project(board_, current_);
            clear_lines();
            just_swapped_ = false;

            if (score_ >= 4)
            {
                state_ = State::Finished;
                return;
            }

            spawn(bag_.poll());
        }

        constexpr void spawn(TetrominoShape shape)
        {
            current_ = Tetromino{
                .shape = shape,
                .rotation = TetrominoRotation::Base,
                .coordinates = { 3, 1 }
            };

            if (is_empty(0, 0))
                return;

            if (is_empty(0, -1))
            {
                --current_.coordinates.y;
                return;
            }

            state_ = State::GameOver;
        }

        constexpr void clear_lines()
        {
            const auto row_begin = current_.coordinates.y + offsets[current_.shape][current_.rotation][Side::Top];
            const auto row_count = std::min(
                    bounding_box_sizes[current_.shape] - offsets[current_.shape][current_.rotation][Side::Bottom],
                    board_.size() - row_begin);

            for (auto row : std::views::iota(row_begin) | std::views::take(row_count))
                if (std::ranges::all_of(board_[row], is_occupied))
                    clear_line(row);
        }

        constexpr void clear_line(uint8_t row)
        {
            std::ranges::copy_backward(board_ | std::views::take(row), board_.begin() + row + 1);
            std::ranges::fill(board_[0], Block::Void);
            ++score_;
        }

        Tetromino current_{};
        std::optional<TetrominoShape> swapped_{};
        TetrominoBag bag_;
        State state_ = State::Playing;
        bool just_swapped_ = false;
        uint16_t score_ = 0;
        Board board_{};
    };
}
