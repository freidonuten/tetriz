#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>


class xoroshiro64ss
{
public:
    using result_type = uint32_t;

    constexpr
    xoroshiro64ss(uint64_t seed)
        : state{
            static_cast<result_type>(seed),
            static_cast<result_type>(seed >> 32)
        }
    { }

    [[nodiscard("Use discard(n) to advance the engine")]]
    constexpr
    auto operator()() -> result_type
    { return next(); }

    constexpr
    void discard(size_t count)
    { while (count--) next(); }

    constexpr
    static auto min() -> result_type
    { return std::numeric_limits<result_type>::min(); }

    constexpr
    static auto max() -> result_type
    { return std::numeric_limits<result_type>::max(); }

private:
    std::array<result_type, 2> state;

    constexpr auto next() -> result_type
    {
        const auto s0 = state[0];
        const auto result = std::rotl(s0 * 0x9E3779BB, 5) * 5;
        const auto s1 = state[1] ^ s0;

        state[0] = std::rotl(s0, 26) ^ s1 ^ (s1 << 9);
        state[1] = std::rotl(s1, 13);

        return result;
    }
};
