#pragma once

#include <array>
#include <string_view>


namespace util
{
template <size_t N>
struct static_string
{
    //NOLINTNEXTLINE clang-tidy modernize-avoid-c-arrays
    constexpr static_string(const char (&string)[N])
        : data(std::to_array(string))
    { }

    constexpr static_string(const std::array<char, N> string)
        : data(string)
    { }

    constexpr operator std::string_view() const
    { return { data.data(), data.size() - 1}; }

    std::array<char, N> data;
};

template <static_string ...Constants>
constexpr
auto is_any(std::string_view value) -> bool
{
    return ((value == Constants) || ...);
}

constexpr
std::string_view ltrim(std::string_view str, std::string_view whitespace = " \r\n\t\v\f")
{
    return str.substr(str.find_first_not_of(whitespace));
}

constexpr
std::string_view rtrim(std::string_view str, std::string_view whitespace = " \r\n\t\v\f")
{
    return str.substr(0, str.find_last_not_of(whitespace) + 1);
}

constexpr
std::string_view trim(std::string_view str, std::string_view whitespace = " \r\n\t\v\f")
{
    return str.substr(str.find_first_not_of(whitespace), str.find_last_not_of(whitespace) + 1);
}
}
