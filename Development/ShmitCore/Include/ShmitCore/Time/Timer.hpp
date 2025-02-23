#pragma once

#include <chrono>

namespace shmit
{
namespace time
{

/**!
 * @brief Single-shot interface for polling expiration over time
 *
 */
class Timer
{
public:
    /**!
     * @brief Checks for expiration
     *
     * @retval true if expired
     * @retval false otherwise
     */
    virtual bool IsExpired() const noexcept = 0;

    /**!
     * @brief Checks for expiration overrun past a threshold set by the implementation
     *
     * @note This is intended to provide an error check for higher level components that may want to treat this
     * situation differently. As such, implementations should set their criteria for this check to whether something out
     * of the ordinary has occured.
     *
     * @retval true if over-expired
     * @retval false otherwise
     */
    virtual bool IsOverExpired() const noexcept = 0;

    /**!
     * @brief Resets the timer
     *
     */
    virtual void Reset() noexcept = 0;
};

/**!
 * @brief Wraps a Timer to add automatic reset functionality
 *
 */
class PeriodicTimer final : public Timer
{
public:
    PeriodicTimer() = delete;

    /**!
     * @brief Constructs a PeriodicTimer
     *
     * @param timer Timer to wrap
     */
    PeriodicTimer(Timer& timer) noexcept;

    /**!
     * @brief Checks for expiration and resets the underlying timer if it has occurred
     *
     * @retval true if expired
     * @retval false otherwise
     */
    bool IsExpired() const noexcept override;

    /**!
     * @brief Checks for over-expiration in the underlying timer
     *
     * @retval true if over-expired
     * @retval false otherwise
     */
    bool IsOverExpired() const noexcept override;

    /**!
     * @brief Resets the underlying timer
     *
     */
    void Reset() noexcept override;

private:
    Timer& m_timer;
};

/**!
 * @brief Basic Timer implementation that utilizes a static clock to poll for expiration
 *
 * @tparam Clock Time source. Must meet the ShmitCore named requirements for Clock.
 */
template<typename Clock>
class BasicTimer : public Timer
{
public:
    using TimePoint = std::chrono::time_point<Clock>; /*! TimePoint specialization used by the timer */
    using Duration  = typename Clock::duration;

    /**!
     * @brief Constructs a zeroed out BasicTimer which will expire immediately
     *
     */
    BasicTimer() noexcept = default;

    /**!
     * @brief Constructs a BasicTimer with a set duration
     *
     * @tparam Denomination Units of time
     * @param duration Timer duration
     */
    template<typename Rep, typename Period>
    BasicTimer(std::chrono::duration<Rep, Period> const& duration) noexcept;

    /**!
     * @copydoc Timer::IsExpired()
     *
     */
    bool IsExpired() const noexcept override;

    /**!
     * @brief Checks for an expiration overrun past the timer's duration a second time
     *
     * @retval true if timer should have expired twice
     * @retval false otherwise
     */
    bool IsOverExpired() const noexcept override;

    /**!
     * @copydoc Timer::Reset()
     *
     */
    void Reset() noexcept override;

    /**!
     * @brief Modifies the timer duration then resets
     *
     * @tparam Denomination Must meet the named requirements
     * @param duration Expiration duration
     */
    template<typename Rep, typename Period>
    void Set(std::chrono::duration<Rep, Period> const& duration) noexcept;

private:
    std::chrono::nanoseconds m_duration_ns;
    TimePoint                m_expire_time;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  BasicTimer constructor definitions                  ////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  Public      ========================================================================================================

template<typename Clock>
template<typename Rep, typename Period>
BasicTimer<Clock>::BasicTimer(std::chrono::duration<Rep, Period> const& duration) noexcept
{
    Set(duration);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  BasicTimer method definitions in alphabetical order                     ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  Public      ========================================================================================================

template<typename Clock>
bool BasicTimer<Clock>::IsExpired() const noexcept
{
    TimePoint now = Clock::now();
    if (now >= m_expire_time)
        return true;

    return false;
}

template<typename Clock>
bool BasicTimer<Clock>::IsOverExpired() const noexcept
{
    // Calculate the time point that is 1 duration's length past the expiration time
    TimePoint overexpire_time {std::chrono::time_point_cast<Duration>(m_expire_time + m_duration_ns)};

    // Calculate the timer's overage and compare against the set duration
    // If the overage exceeds the original length of the timer, it is over-expired
    TimePoint now        = Clock::now();
    Duration  overage_ns = std::chrono::duration_cast<Duration>(now - m_expire_time);
    if (overage_ns > m_duration_ns)
        return true;

    return false;
}

template<typename Clock>
void BasicTimer<Clock>::Reset() noexcept
{
    // Use the previous expiration time as the start of the new period
    // If the timer is over-expired, or not expired at all, use the current time instead
    TimePoint reset_start = m_expire_time;
    if (!IsExpired() || IsOverExpired())
        reset_start = Clock::now();

    m_expire_time = std::chrono::time_point_cast<Duration>(reset_start + m_duration_ns);
}

template<typename Clock>
template<typename Rep, typename Period>
void BasicTimer<Clock>::Set(std::chrono::duration<Rep, Period> const& duration) noexcept
{
    m_duration_ns = std::chrono::duration_cast<Duration>(duration);
    Reset();
}

} // namespace time
} // namespace shmit
