#pragma once

#include <cstdint>
#include <string>


struct Configuration
{
    std::string host;
    uint16_t port;
    uint32_t log_level;
};

