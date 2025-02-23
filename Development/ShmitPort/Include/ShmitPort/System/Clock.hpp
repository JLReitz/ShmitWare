#pragma once

#include <zephyr/kernel.h>

#include <chrono>
#include <type_traits>

#include "ShmitCore/StdTypes.hpp"

namespace shmit
{
namespace port
{
namespace system
{

struct SteadyClock
{
#ifdef CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME

    using duration = std::chrono::milliseconds;

#else

    constexpr static int64_t kSteadyClockStaticFrequencyHz {CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC};
    using duration = std::chrono::duration<uint64_t, std::ratio<1L, kSteadyClockStaticFrequencyHz>>;

#endif

    using period = duration::period;
    using rep    = duration::rep;

    using time_point = std::chrono::time_point<SteadyClock>;

    static bool const is_steady {true};

    static time_point now();
};

using WallClock = SteadyClock;

} // namespace system
} // namespace port
} // namespace shmit