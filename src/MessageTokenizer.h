#pragma once

#include <string>

class MessageTokenizer
{
public:
    MessageTokenizer(std::string_view message);

    [[nodiscard]] auto is_done() const -> bool;
    [[nodiscard]] auto leading_char() const -> char;

    auto next_uint() -> int32_t;
    auto next_string() -> std::string;

    auto operator>>(std::string& data) -> MessageTokenizer&
    {
        data = next_string();
        return *this;
    }

    auto operator>>(int32_t& data) -> MessageTokenizer&
    {
        data = next_uint();
        return *this;
    }

    auto operator>>(char& header) -> MessageTokenizer&
    {
        header = lead;
        return *this;
    }

private:
    [[nodiscard]] auto is_terminated() const -> bool;
    [[nodiscard]] auto is_exhausted() const -> bool;

    void consumeDelimiters();

    int32_t tok_start{};
    int32_t tok_end = 1;
    std::string_view base;
    char lead;
};
