#pragma once

#include <cstdint>

#include "magic_enum/magic_enum_utility.hpp"


namespace tetriz
{
    enum class TetrominoRotation : uint8_t
    {
        Base, Right, Double, Left
    };

    constexpr TetrominoRotation next_left(TetrominoRotation rotation)
    {
        return magic_enum::enum_next_value_circular(rotation);
    }

    constexpr TetrominoRotation next_right(TetrominoRotation rotation)
    {
        return magic_enum::enum_prev_value_circular(rotation);
    }
}
