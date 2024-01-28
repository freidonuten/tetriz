#include "MessageTokenizer.h"
#include "constants.h"



MessageTokenizer::MessageTokenizer(std::string_view message)
    : base(message)
    , lead(base.front())
{
    // trim start
    consumeDelimiters();
    tok_start = tok_end;
}

auto MessageTokenizer::is_terminated() const -> bool
{
    return base.at(base.length() - 1) == MSG_SEP;
}

auto MessageTokenizer::is_exhausted() const -> bool
{
    return tok_start >= base.length() - 1;
}

auto MessageTokenizer::leading_char() const -> char
{
    return lead;
}

auto MessageTokenizer::next_uint() -> int32_t
{
    constexpr auto numeric_base = 10;
    auto result = 0;

    // build result
    while (tok_end < base.length())
    {
        const auto chr = base.at(tok_end);

        if (isdigit(chr) != 0)
        {
            result = result * numeric_base + chr - '0';
            ++tok_end;
            continue;
        }

        break;
    }

    // return result
    if (tok_end != tok_start)
    {
        consumeDelimiters();
        tok_start = tok_end;
        return result;
    } 

    return -1;
}

auto MessageTokenizer::next_string() -> std::string {

    // iterate until delim/end of message
    while (tok_end < base.length())
    {
        const auto chr = base.at(tok_end);

        if (chr == MSG_DELIM || chr == MSG_SEP)
        {
            break;
        }

        ++tok_end;
    }

    // build result and set new boundaries
    const auto result = base.substr(tok_start, tok_end - tok_start);
    consumeDelimiters();
    tok_start = tok_end;

    return std::string(result);
}

void MessageTokenizer::consumeDelimiters()
{
    while (base.at(tok_end) == MSG_DELIM)
    {
        ++tok_end;
    }
}

auto MessageTokenizer::is_done() const -> bool
{
    return is_terminated() && is_exhausted();
}



