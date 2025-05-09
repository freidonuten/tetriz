#pragma once

#include "coordinates.hpp"
#include "tetromino_shape.hpp"
#include "rotation.hpp"


namespace tetriz
{
    struct Tetromino
    {
        TetrominoShape shape = TetrominoShape::T;
        TetrominoRotation rotation = TetrominoRotation::Base;
        Coordinates coordinates{};
    };

    constexpr auto operator+(Tetromino lhs, Coordinates rhs) -> Tetromino
    {
        return {
            .shape = lhs.shape,
            .rotation = lhs.rotation,
            .coordinates = lhs.coordinates + rhs
        };
    }

    constexpr auto operator+=(Tetromino& lhs, Coordinates rhs) -> Tetromino&
    {
        lhs.coordinates += rhs;
        return lhs;
    }
}
