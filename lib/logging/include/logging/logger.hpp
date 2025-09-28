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

auto log_level() -> Severity;
void set_log_level(Severity);

auto log_stream() -> std::ostream&;
void set_log_stream(std::ostream&);


namespace detail
{
    template <Severity S, typename ...Args>
    void log(std::format_string<Args...> format, Args&& ...args)
    {
        if (static_cast<int>(S) <= static_cast<int>(log_level()))
        {
            auto& ostream = log_stream();
            std::print(ostream, "[{}]: ", magic_enum::enum_name<S>().substr(0, 3));
            std::println(ostream, format, std::forward<Args>(args)...);
            std::flush(ostream);
        }
    }
}

template <typename ...Args>
void log_error(std::format_string<Args...> format, Args&& ...args)
{
    detail::log<Severity::Error>(format, std::forward<Args>(args)...);
}

template <typename ...Args>
void log_warning(std::format_string<Args...> format, Args&& ...args)
{
    detail::log<Severity::Warning>(format, std::forward<Args>(args)...);
}

template <typename ...Args>
void log_info(std::format_string<Args...> format, Args&& ...args)
{
    detail::log<Severity::Info>(format, std::forward<Args>(args)...);
}

template <typename ...Args>
void log_debug(std::format_string<Args...> format, Args&& ...args)
{
    detail::log<Severity::Debug>(format, std::forward<Args>(args)...);
}

template <typename ...Args>
void log_trace(std::format_string<Args...> format, Args&& ...args)
{
    detail::log<Severity::Trace>(format, std::forward<Args>(args)...);
}
