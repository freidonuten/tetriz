#pragma once

#include "logger.hpp"
#include <cstdint>
#include <vector>
#include <string>


class Response
{
public:
    enum class Status : bool
    {
        FAIL, SUCCESS
    };

    Response() = default;
    Response(Status status)
        : status(static_cast<bool>(status))
    {}

    template <typename ...Args>
    Response(Args&& ...args)
    { (add(std::forward<Args>(args)), ...); }

    auto add_int(uint32_t value) -> Response&;
    auto add_string(const std::string& value) -> Response&;
    auto set_fail() -> Response&;
    auto set_success() -> Response&;
    auto to_string() -> std::string;

    operator std::string() { return to_string(); }

    template <typename T>
    auto add(T&& value) -> Response&
    {
        using T_decayed = std::decay_t<T>;

        if constexpr (std::is_same_v<T_decayed, bool>)
        {
            status = value;
        }
        else if constexpr (std::is_integral_v<T_decayed>)
        {
            add_int(value);
        }
        else if constexpr (std::is_convertible_v<T_decayed, std::string>)
        {
            add_string(std::forward<T>(value));
        }
        else
        {
            log_warning("Failed to add data to response");
        }

        return *this;
    }

    template <typename ...Args>
    auto add(Args&& ...args) -> Response&
    {
        return (add(std::forward<Args>(args)), ...);
    }

    static constexpr auto fail = Status::FAIL;
    static constexpr auto success = Status::SUCCESS;

private:
    bool status = true;
    std::vector<uint32_t> integers;
    std::vector<std::string> strings;
};
