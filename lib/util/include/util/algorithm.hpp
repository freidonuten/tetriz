#pragma once

#include <iterator>


namespace utl
{
    template <std::forward_iterator InputIt>
    [[nodiscard]] constexpr
    auto find_not(InputIt begin, InputIt end, const auto& elem) -> InputIt
    {
        for (; begin != end && *begin == elem; ++begin);
        return begin;
    }

    namespace ranges
    {
        template <std::ranges::forward_range Range>
        [[nodiscard]] constexpr
        auto find_not(Range&& range, const auto& elem)
            -> std::forward_iterator auto
        {
            return utl::find_not(std::ranges::begin(range), std::ranges::end(range), elem);
        }
    }
}
