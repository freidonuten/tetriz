#pragma once

#include "engine/coordinates.hpp"
#include "engine/tetromino_shape.hpp"
#include "engine/rotation.hpp"


namespace tetriz
{
    struct Tetromino
    {
        TetrominoShape shape = TetrominoShape::T;
        TetrominoRotation rotation = TetrominoRotation::Base;
        TetrominoCoordinates coordinates{};
    };
}
