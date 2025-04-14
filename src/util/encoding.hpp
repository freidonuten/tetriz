#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <ranges>


[[nodiscard]]
constexpr auto hexdump_byte(uint8_t byte) -> std::array<char, 2>
{
    static constexpr auto digits = std::to_array({
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'A', 'B',
        'C', 'D', 'E', 'F'
    });

    return { digits[(byte & 0xF0) >> 4], digits[byte & 0x0F] };
}

[[nodiscard]]
constexpr auto hexdump(std::span<const uint8_t> range) -> std::string
{
    return range
        | std::views::transform(hexdump_byte)
        | std::views::join
        | std::ranges::to<std::string>();
}
