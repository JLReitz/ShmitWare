#pragma once

#include "Inbound.hpp"
#include "_Detail/Ingress.hpp"

#include "ShmitCore/Data/Decode.hpp"
#include "ShmitCore/Data/Footprint.hpp"
#include "ShmitCore/IO/Input.hpp"
#include "ShmitCore/Span.hpp"

#include <chrono>
#include <cstring>
#include <type_traits>

namespace shmit
{
namespace io
{
namespace session
{

template<typename T>
class Ingress final : public Input<T>, public _detail::IngressBase
{
public:
    /// @brief Type that is read from the Ingress instance
    using value_type = typename Input<T>::value_type;

    /**!
     * @brief Initializing constructor. Connects Ingress to an Inbound session buffer.
     *
     * @param[in] session Reference to Inbound session buffer that the Ingress will connect with
     */
    Ingress(Inbound& session) noexcept;

    // Ingress is not trivially constructible

    Ingress() = delete;

    // Ingress is trivially copy/move constructible and destructible

    Ingress(Ingress const& copy) = default;
    Ingress(Ingress&& move)      = default;

    virtual ~Ingress() = default;

    // Ingress is trivially copy and move assignable

    Ingress& operator=(Ingress const& copy) = default;
    Ingress& operator=(Ingress&& move)      = default;

    virtual BinaryResult Get(value_type& data) noexcept override;

    BinaryResult Get(value_type& data, std::chrono::microseconds timeout) noexcept;

private:
    /// @brief Reference to connected session
    Inbound& m_buffer;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace metafunction declarations              ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**!
 * @brief Checks if type is an Ingress specialization
 *
 * @tparam T Potential Ingress type
 */
template<typename T>
using is_ingress = typename std::is_base_of<_detail::IngressBase, T>::type;

/**!
 * @brief Convenience alias to access the returned type of is_ingress
 *
 * @tparam T Potential Ingress type
 */
template<typename T>
using is_ingress_t = typename is_ingress<T>::type;

/**!
 * @brief Convenience alias to access the returned value of is_ingress
 *
 * @tparam T Potential Ingress type
 */
template<typename T>
constexpr static bool is_ingress_v {is_ingress<T>::value};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Ingress constructor definitions             ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  Public      ========================================================================================================

template<typename T>
Ingress<T>::Ingress(Inbound& session) noexcept : m_buffer {session}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Ingress method definitions in alphabetical order                ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  Public      ========================================================================================================

template<typename T>
BinaryResult Ingress<T>::Get(value_type& data) noexcept
{
    return Get(data, std::chrono::microseconds::zero());
}

template<typename T>
BinaryResult Ingress<T>::Get(value_type& data, std::chrono::microseconds timeout) noexcept
{
    constexpr static size_t kDataSizeBytes {data::footprint_size_bytes_v<value_type>};

    // Guard against underflowing the Inbound buffer
    if (m_buffer.InputBytesAvailable() < kDataSizeBytes)
        return BinaryResult::Failure();

    // Pack Transference with apropriately sized, empty buffer and pass to session for request
    uint8_t       encoded_buffer[kDataSizeBytes];
    Span<uint8_t> encoded_span {encoded_buffer, kDataSizeBytes};
    static_cast<void>(std::memset(encoded_buffer, 0U, kDataSizeBytes)); // Avoid unused return warning

    Inbound::Result request_result {m_buffer.Request(encoded_span, timeout)};
    if (request_result.IsFailure())
        return BinaryResult::Failure();

    // Transference has valid data to decode
    // Perform decoding, return result
    size_t       bits_decoded {0U};
    BinaryResult decode_result {data::decode(encoded_buffer, bits_decoded, data)};
    return (decode_result.IsSuccess() ? BinaryResult::Success() : BinaryResult::Failure());
}

} // namespace session
} // namespace io
} // namespace shmit