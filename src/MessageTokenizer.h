#pragma once

#include <memory>
#include <string>

class MessageTokenizer
{
public:
    MessageTokenizer(std::string& message);

    [[nodiscard]] auto is_done() const -> bool;
    [[nodiscard]] auto leading_char() const -> char;

    auto next_uint() -> int32_t;
    auto next_string() -> std::string;

private:
    [[nodiscard]] auto is_terminated() const -> bool;
    [[nodiscard]] auto is_exhausted() const -> bool;

    void consumeDelimiters();

    int32_t tok_start{};
    int32_t tok_end = 1;
    std::string base;
    char lead;
};
