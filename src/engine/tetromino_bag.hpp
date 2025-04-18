#pragma once

#include "magic_enum/magic_enum.hpp"

#include "engine/tetromino_shape.hpp"
#include <algorithm>
#include <generator>
#include <random>


namespace tetriz
{
    inline std::random_device rd;
    inline std::mt19937 gen{rd()};

    class TetrominoBag
    {
    public:
        constexpr TetrominoBag()
        {
            fill(left_half());
            fill(right_half());
        }

        auto poll() -> TetrominoShape
        {
            const auto next = bag_.front();
            std::shift_left(bag_.begin(), bag_.end(), 1);

            if (++current_shift_ == 7)
            {
                fill(right_half());
                current_shift_ = 0;
            }

            return next;
        }

        template <size_t N>
        requires (N <= 7)
        constexpr auto peek() const
        {
            return bag_ | std::views::take(N);
        }

    private:
        constexpr void fill(std::span<TetrominoShape, 7> range)
        {
            std::ranges::copy(magic_enum::enum_values<TetrominoShape>(), range.begin());
            std::ranges::shuffle(range, gen);
        }

        constexpr auto left_half() -> std::span<TetrominoShape, 7>
        {
            return std::span(bag_).first<7>();
        }

        constexpr auto right_half() -> std::span<TetrominoShape, 7>
        {
            return std::span(bag_).last<7>();
        }

        uint8_t current_shift_;
        std::array<tetriz::TetrominoShape, 14> bag_;
    };
}
