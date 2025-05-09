#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <span>


namespace tetriz::proto
{

template <typename ...Ts>
constexpr auto pack_size = (sizeof(Ts) + ...);

template <typename T>
constexpr auto byte_range(const T& value) -> std::span<const uint8_t>
{
    return { reinterpret_cast<const uint8_t*>(&value), sizeof(value) };
}

template <typename ...Ts>
[[nodiscard]]
constexpr auto serialize(Ts&& ...ts)
{
    auto buffer = std::array<uint8_t, pack_size<Ts...>>{};
    auto output = buffer.begin();

    (..., (output = std::ranges::copy(byte_range(ts), output).out));

    return buffer;
}

template <typename T>
[[nodiscard]]
constexpr auto pop_from(std::span<const uint8_t>& raw) -> T
{
    assert(sizeof(T) <= raw.size());

    const auto result = *reinterpret_cast<const T*>(raw.data());
    raw = raw.subspan(sizeof(T));

    return result;
}
}
