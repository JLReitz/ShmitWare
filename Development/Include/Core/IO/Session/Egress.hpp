#pragma once

#include "Outbound.hpp"
#include "_Detail/Egress.hpp"

#include "Core/Data/Encode.hpp"
#include "Core/Data/Footprint.hpp"
#include "Core/IO/Output.hpp"
#include "Core/Platform/Clock.hpp"
#include "Core/Span.hpp"

#include <chrono>
#include <cstring>
#include <type_traits>

namespace shmit
{
namespace io
{
namespace session
{

/**!
 * @brief Mirriam-Webster defines "egress" as a place or means of going out. Take any object that is alive and put it to
 * an Egress instance to yeet a copy of it in to the world.
 *
 * @tparam T Any type. CV and reference qualifiers are removed.
 *
 * @note - Egress fits best in to use cases as an asynchronous output where there is no expected return data or complex
 * failure cases
 */
template<typename T>
class Egress : public Output<T>, public _detail::EgressBase
{
public:
    /// @brief Type that is put to the Egress instance
    using value_type = typename Output<T>::value_type;

    /**!
     * @brief Initializing constructor. Connects Egress to an Outbound session buffer.
     *
     * @param[in] buffer Reference to Outbound session buffer that the Egress will connect with
     */
    Egress(Outbound& buffer) noexcept;

    // Egress is not trivially constructible

    Egress() = delete;

    // Egress is trivially copy/move constructible and destructible

    Egress(Egress const& copy) = default;
    Egress(Egress&& move)      = default;

    virtual ~Egress() = default;

    // Egress is trivially copy and move assignable

    Egress& operator=(Egress const& copy) = default;
    Egress& operator=(Egress&& move)      = default;

    /**!
     * @brief Post an object's data to the connected Outbound buffer with no blocking delay
     *
     * @param[in] data Reference to an object to put to the Egress
     * @retval BinaryResult::kSuccessCode if successful
     * @retval BinaryResult::kFailureCode otherwise
     */
    virtual BinaryResult Put(value_type const& data) noexcept override;

    /**!
     * @brief Post an object's data to the connected Outbound buffer, blocking for a duration or until the transference
     * is complete, whichever finishes first
     *
     * @param[in] data Reference to an object to put to the Egress
     * @param[in] duration Maximum time that will be spent attempting to post data
     * @retval BinaryResult::kSuccessCode if successful
     * @retval BinaryResult::kFailureCode otherwise
     */
    BinaryResult Put(value_type const& data, std::chrono::microseconds duration) noexcept;

private:
    /// @brief Reference to connected buffer
    Outbound& m_buffer {};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace metafunction declarations              ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**!
 * @brief Checks if type is an Egress specialization
 *
 * @tparam T Potential Egress type
 */
template<typename T>
using is_egress = typename std::is_base_of<_detail::EgressBase, T>::type;

/**!
 * @brief Convenience alias to access the returned type of is_egress
 *
 * @tparam T Potential Egress type
 */
template<typename T>
using is_egress_t = typename is_egress<T>::type;

/**!
 * @brief Convenience alias to access the returned value of is_egress
 *
 * @tparam T Potential Egress type
 */
template<typename T>
constexpr static bool is_egress_v {is_egress<T>::value};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Egress constructor definitions              ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  Public      ========================================================================================================

template<typename T>
Egress<T>::Egress(Outbound& buffer) noexcept : m_buffer {buffer}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Egress method definitions in alphabetical order             ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  Public      ========================================================================================================

template<typename T>
BinaryResult Egress<T>::Put(value_type const& data) noexcept
{
    return Put(data, std::chrono::microseconds::zero());
}

template<typename T>
BinaryResult Egress<T>::Put(value_type const& data, std::chrono::microseconds duration) noexcept
{
    constexpr static size_t kDataSizeBytes {data::footprint_size_bytes_v<value_type>};

    // Guard against overflowing the Outbound buffer
    if (m_buffer.OutputBytesAvailable() < kDataSizeBytes)
        return BinaryResult::Failure();

    // Save time started
    auto start_us {platform::Clock::now().time_since_epoch()};

    // Encode data in byte buffer
    uint8_t       encoded_buffer[kDataSizeBytes];
    Span<uint8_t> encoded_span {encoded_buffer, kDataSizeBytes};
    static_cast<void>(std::memset(encoded_buffer, 0U, kDataSizeBytes)); // Avoid unused return warning

    size_t       bits_encoded {0U};
    BinaryResult encode_result {data::encode(data, encoded_span, bits_encoded)};
    if (!encode_result.IsSuccess())
        return encode_result;

    // Calculate the duration that encoding took
    auto encoding_end_us {platform::Clock::now().time_since_epoch()};
    auto encoding_duration {std::chrono::duration_cast<std::chrono::microseconds>(encoding_end_us - start_us)};
    if (encoding_duration > duration)
        duration = std::chrono::microseconds::zero();
    else
        duration -= encoding_duration;

    // Pack span in to a Transference and post to Outbound buffer
    Outbound::Result post_result {m_buffer.Post(span_cast<uint8_t const>(encoded_span), duration)};
    return (post_result.IsSuccess() ? BinaryResult::Success() : BinaryResult::Failure());
}

} // namespace session
} // namespace io
} // namespace shmit