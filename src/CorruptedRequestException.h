#pragma once

#include <stdexcept>
#include <string>


class SerializerError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;

    [[nodiscard]] constexpr
    auto what() const noexcept -> const char* override
    {
        return "Corrupted request!";
    }
};
