#pragma once

#include <iostream>
#include <ostream>

enum class Severity
{
    Error,
    Warning,
    Info,
    Debug,
    Trace
};

inline constexpr auto log_level = Severity::Info;

namespace detail
{
    consteval auto to_string(Severity severity) -> std::string_view
    {
        using enum Severity;

        switch (severity)
        {
            case Error: return "Err";
            case Warning: return "Wrn";
            case Info: return "Inf";
            case Debug: return "Dbg";
            case Trace: return "Trc";
        }

        return "???";
    }

    template <Severity S, typename ...Args>
    void log(std::ostream& outstream, std::format_string<Args...> format, Args&& ...args)
    {
        if constexpr (static_cast<int>(log_level) <= static_cast<int>(S))
        {
            std::print(outstream, "[{}]: ", to_string(S));
            std::println(outstream, format, std::forward<Args>(args)...);
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
