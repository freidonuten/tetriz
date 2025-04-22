#pragma once

#include <algorithm>
#include <generator>
#include <random>

#include "magic_enum/magic_enum.hpp"

#include "engine/tetromino_shape.hpp"


namespace tetriz
{
    class TetrominoBag
    {
    public:
        constexpr TetrominoBag(uint32_t seed)
            : generator_(seed)
        {
            fill(left_half());
            fill(right_half());
        }

        constexpr auto poll() -> TetrominoShape
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
            auto result = std::array<TetrominoShape, N>{};
            std::ranges::copy_n(bag_.begin(), N, result.begin());
            return result;
        }

    private:
        constexpr void fill(std::span<TetrominoShape, 7> range)
        {
            std::ranges::copy(magic_enum::enum_values<TetrominoShape>(), range.begin());
            std::ranges::shuffle(range, generator_);
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
        std::mt19937 generator_;
        std::array<TetrominoShape, 14> bag_;
    };
}
