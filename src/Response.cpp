#include "Response.h"
#include "constants.h"

auto Response::add_int(uint32_t value) -> Response&
{
    integers.push_back(value);
    return *this;
}

auto Response::add_string(const std::string& value) -> Response&
{
    strings.push_back(value);
    return *this;
}

auto Response::set_fail() -> Response&
{
    success = false;
    return *this;
}

auto Response::set_success() -> Response&
{
    success = true;
    return *this;
}

auto Response::to_string() -> std::string
{
    auto response = std::string{ success ? MSG_SUCCESS : MSG_FAIL };

    for (uint32_t integer: integers)
    {
        response.append(" " + std::to_string(integer));
    }

    for (const auto& string : strings)
    {
        response.append(" " + string);
    }

    return response + MSG_SEP;
}


