#pragma once

#include <cstdint>

#include "magic_enum/magic_enum_utility.hpp"


namespace tetriz
{
    enum class TetrominoRotation : uint8_t
    {
        Base, Right, Double, Left
    };

    constexpr auto next_left(TetrominoRotation rotation) -> TetrominoRotation
    {
        return magic_enum::enum_next_value_circular(rotation);
    }
}
