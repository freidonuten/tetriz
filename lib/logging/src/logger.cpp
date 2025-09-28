#include "logging/logger.hpp"

auto log_level_ = Severity::Info;
auto* ostream_ = &std::cout;

auto log_level() -> Severity
{
    return log_level_;
}

void set_log_level(Severity severity)
{
    log_level_ = severity;
}

auto log_stream() -> std::ostream&
{
    return *ostream_;
}

void set_log_stream(std::ostream& ostream)
{
    ostream_ = &ostream;
}

