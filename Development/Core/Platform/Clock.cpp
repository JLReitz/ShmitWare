#include <Core/Platform/Clock.hpp>

namespace shmit
{
namespace platform
{

#ifndef NATIVE_SHMIT

extern Clock::duration get_platform_clock_duration_since_epoch() noexcept;

#else

Clock::duration get_platform_clock_duration_since_epoch() noexcept
{
    return std::chrono::duration_cast<Clock::duration>(std::chrono::system_clock::now().time_since_epoch());
}

#endif

Clock::time_point Clock::now() // Static method
{
    return time_point {get_platform_clock_duration_since_epoch()};
}

} // namespace platform
} // namespace shmit
