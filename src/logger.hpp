#pragma once

#include <iostream>
#include <ostream>

#include "magic_enum/magic_enum.hpp"


enum class Severity
{
    Error = 1,
    Warning,
    Info,
    Debug,
    Trace
};

inline auto log_level = Severity::Info;

namespace detail
{
    template <Severity S, typename ...Args>
    void log(std::ostream& outstream, std::format_string<Args...> format, Args&& ...args)
    {
        if (static_cast<int>(S) <= static_cast<int>(log_level))
        {
            std::print(outstream, "[{}]: ", magic_enum::enum_name<S>().substr(0, 3));
            std::println(outstream, format, std::forward<Args>(args)...);
            std::flush(outstream);
        }
    }
}

template <typename ...Args>
void log_error(std::format_string<Args...> format, Args&& ...args)
{
    detail::log<Severity::Error>(std::cerr, format, std::forward<Args>(args)...);
}

template <typename ...Args>
void log_warning(std::format_string<Args...> format, Args&& ...args)
{
    detail::log<Severity::Warning>(std::cout, format, std::forward<Args>(args)...);
}

template <typename ...Args>
void log_info(std::format_string<Args...> format, Args&& ...args)
{
    detail::log<Severity::Info>(std::cout, format, std::forward<Args>(args)...);
}

template <typename ...Args>
void log_debug(std::format_string<Args...> format, Args&& ...args)
{
    detail::log<Severity::Debug>(std::cout, format, std::forward<Args>(args)...);
}

template <typename ...Args>
void log_trace(std::format_string<Args...> format, Args&& ...args)
{
    detail::log<Severity::Trace>(std::cout, format, std::forward<Args>(args)...);
}
