#pragma once

#include <chrono>

namespace shmit
{
namespace platform
{

struct Clock
{
    using duration = std::chrono::microseconds;
    using period   = duration::period;
    using rep      = duration::rep;

    using time_point = std::chrono::time_point<Clock>;

    static bool const is_steady {false};

    static time_point now();
};

} // namespace platform
} // namespace shmit
