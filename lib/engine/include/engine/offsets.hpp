#pragma once

#include "magic_enum/magic_enum_containers.hpp"

#include "rotation.hpp"
#include "tetromino_shape.hpp"


namespace tetriz
{
    enum class Side
    {
        Top, Bottom, Left, Right
    };

    constexpr inline auto offsets =
        magic_enum::containers::array<TetrominoShape,
        magic_enum::containers::array<TetrominoRotation,
        magic_enum::containers::array<Side, uint8_t>>>
    {{{
        //  T  B  L  R
        {{{{1, 2, 0, 0}, {0, 0, 2, 1}, {2, 1, 0, 0}, {0, 0, 1, 2}}}}, // I
        {{{{0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}}}}, // J
        {{{{0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}}}}, // L
        {{{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}}}, // O
        {{{{0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}}}}, // S
        {{{{0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}}}}, // T
        {{{{0, 1, 0, 0}, {0, 0, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 1}}}}, // Z
    }}};
};
