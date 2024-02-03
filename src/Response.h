#pragma once

#include <vector>
#include <string>
#include <memory>


class Response
{
public:
    Response() = default;
    Response(bool status) : status(status) {}

    auto add_int(uint32_t value) -> Response&;
    auto add_string(const std::string& value) -> Response&;
    auto set_fail() -> Response&;
    auto set_success() -> Response&;
    auto to_string() -> std::string;

    static constexpr auto fail = false;
    static constexpr auto success = true;
private:
    bool status = true;
    std::vector<uint32_t> integers;
    std::vector<std::string> strings;
};

