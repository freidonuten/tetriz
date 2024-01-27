#pragma once

#include <vector>
#include <string>
#include <memory>


class Response
{
public:
    auto add_int(uint32_t value) -> Response&;
    auto add_string(const std::string& value) -> Response&;
    auto set_fail() -> Response&;
    auto set_success() -> Response&;
    auto to_string() -> std::string;

private:
    bool success = true;
    std::vector<uint32_t> integers;
    std::vector<std::string> strings;
};

