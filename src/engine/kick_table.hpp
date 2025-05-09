#pragma once

#include <array>
#include <cstdint>
#include <generator>

#include "engine/rotation.hpp"
#include "engine/tetromino_shape.hpp"
#include "magic_enum/magic_enum_containers.hpp"


namespace tetriz
{
    using KickTable = magic_enum::containers::array<
        TetrominoRotation, std::array<std::pair<int8_t, int8_t>, 5>>;

    constexpr auto kick_table_other = KickTable{{{
        {{{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}}},
        {{{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}}},
        {{{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}}},
        {{{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}}},
    }}};

    constexpr auto kick_table_I = KickTable{{{
        {{{ 0, 0}, {-2, 0}, { 1, 0}, {-2, 1}, { 1,-2}}},
        {{{ 0, 0}, {-1, 0}, { 2, 0}, {-1,-2}, { 2, 1}}},
        {{{ 0, 0}, { 2, 0}, {-1, 0}, { 2,-1}, {-1, 2}}},
        {{{ 0, 0}, { 1, 0}, {-2, 0}, { 1, 2}, {-2,-1}}},
    }}};

    constexpr auto kick_offsets(TetrominoShape shape, TetrominoRotation rotation)
    {
        return (shape == TetrominoShape::I ? kick_table_I : kick_table_other)[rotation];
    }
}
