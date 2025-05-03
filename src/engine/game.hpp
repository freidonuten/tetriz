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
        None,
        Left,
        Right,
        Down,
        Up
    };

    constexpr inline auto moves = magic_enum::containers::array<Direction, Coordinates>{
        Coordinates{ .x =  0, .y =  0 },
        Coordinates{ .x = -1, .y =  0 },
        Coordinates{ .x =  1, .y =  0 },
        Coordinates{ .x =  0, .y =  1 },
        Coordinates{ .x =  0, .y = -1 },
    };

    constexpr inline auto offset_none = moves[Direction::None];
    constexpr inline auto offset_down = moves[Direction::Down];
    constexpr inline auto offset_up = moves[Direction::Up];

    enum class State : uint8_t
    {
        Playing,
        Finished,
        GameOver
    };

    class Game
    {
    public:
        constexpr Game(uint32_t seed) : Game(seed, 40) { }

        constexpr Game(uint32_t seed, uint16_t score_goal)
            : bag_(seed)
            , score_goal_(score_goal)
        {
            spawn(bag_.poll());
        }

        constexpr void move(Direction direction)
        {
            const auto move = tetriz::moves[direction];
            
            if (is_empty(move))
            {
                current_.coordinates += move;
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
            while (is_empty(offset_down))
                current_.coordinates.y += 1;

            lock();
        }

        constexpr void rotate()
        {
            current_.rotation = next_left(current_.rotation);

            for (const auto offset : kick_offsets(current_.shape, current_.rotation))
            {
                if (is_empty(offset))
                {
                    current_.coordinates += offset;
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
        constexpr auto is_empty(Coordinates offset) const -> bool
        {
            return projects(board_, current_ + offset);
        }

        constexpr void lock()
        {
            project(board_, current_);
            clear_lines();
            just_swapped_ = false;
            spawn(bag_.poll());

            if (score_ >= score_goal_)
            {
                state_ = State::Finished;
                return;
            }
        }

        constexpr void spawn(TetrominoShape shape)
        {
            current_ = Tetromino{
                .shape = shape,
                .rotation = TetrominoRotation::Base,
                .coordinates = { 3, 1 }
            };

            for (const auto offset : {offset_none, offset_up})
            {
                if (is_empty(offset))
                {
                    current_.coordinates += offset;
                    return;
                }
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
                if (std::ranges::all_of(board_[row], std::bind_front(std::not_equal_to{}, Block::Void)))
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
        State state_ = State::Playing;
        bool just_swapped_ = false;
        uint16_t score_ = 0;
        uint16_t score_goal_ = 40;
        Board board_{};
        TetrominoBag bag_;
    };
}
