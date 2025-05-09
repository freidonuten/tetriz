#pragma once

#include <cstdint>


namespace tetriz
{
    struct Coordinates
    {
        int8_t x = 0;
        int8_t y = 0;
    };

    constexpr
    auto operator+(Coordinates lhs, Coordinates rhs) -> Coordinates
    {
        return {
            .x = static_cast<int8_t>(lhs.x + rhs.x),
            .y = static_cast<int8_t>(lhs.y + rhs.y)
        };
    }

    constexpr
    auto operator+=(Coordinates& lhs, Coordinates rhs) -> Coordinates&
    {
        return (lhs = lhs + rhs);
    }
}
