#pragma once

#include <chrono>

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<int32_t, std::centi>;

template <typename T, typename R>
auto normalize_duration(std::chrono::duration<T, R> value)
{
    return std::chrono::duration_cast<Duration>(value);
}

