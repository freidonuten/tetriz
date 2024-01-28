#pragma once

#include <cstdint>
#include <memory>
#include "MessageTokenizer.h"
#include "CorruptedRequestException.h"

namespace ProtoUtil
{

    auto uint_query(std::string& msg) -> uint32_t
    {
        MessageTokenizer mtok(msg);
        auto integer = mtok.next_uint();

        if (!mtok.is_done() || integer == -1)
        {
            throw SerializerError("");
        }

        return integer;
    }

    auto string_query(std::string& msg) -> std::string
    {
        MessageTokenizer mtok = MessageTokenizer(msg);
        auto string = mtok.next_string();

        if (string.empty() || !mtok.is_done())
        {
            throw SerializerError("");
        }

        return string;
    }

    void zero_arg_query(std::string& msg)
    {
        if (!MessageTokenizer(msg).is_done())
        {
            throw SerializerError("");
        }
    }

}
