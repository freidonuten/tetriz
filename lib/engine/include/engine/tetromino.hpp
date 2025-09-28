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

    constexpr auto operator==(Tetromino lhs, Tetromino rhs) -> bool
    {
        return lhs.coordinates == rhs.coordinates
            && lhs.shape == rhs.shape
            && lhs.rotation == rhs.rotation;
    }
}

namespace std
{
    template <>
    struct hash<tetriz::Tetromino>
    {
        [[nodiscard]]
        constexpr auto operator()(const tetriz::Tetromino& tetromino) const noexcept -> size_t
        {
            return std::bit_cast<uint32_t>(tetromino);
        }
    };
}
