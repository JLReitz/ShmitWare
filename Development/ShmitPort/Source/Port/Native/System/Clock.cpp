#include "ShmitPort/System/Clock.hpp"

#include <zephyr/kernel.h>

#include <chrono>

namespace shmit
{
namespace port
{
namespace system
{

SteadyClock::time_point SteadyClock::now() // Static method
{
    rep count {duration::zero().count()};
#ifdef CONFIG_TIMER_READS_ITS_FREQUENCY_AT_RUNTIME
    count = static_cast<rep>(k_uptime_get());
#elif CONFIG_TIMER_HAS_64BIT_CYCLE_COUNTER
    count = static_cast<rep>(k_cycle_get_64());
#else
    count = static_cast<rep>(k_cycle_get_32());
#endif

    duration duration_since_epoch {count};
    return time_point {duration_since_epoch};
}

} // namespace system
} // namespace port
} // namespace shmit