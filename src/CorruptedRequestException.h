#pragma once

#include <bits/exception.h>
#include <string>


class CorruptedRequestException : public std::exception
{
public:
    [[nodiscard]] constexpr
    auto what() const noexcept -> const char* override
    {
        return "Corrupted request!";
    }
};
