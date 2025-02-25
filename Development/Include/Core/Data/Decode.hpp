#pragma once

#include "Footprint.hpp"

#include "Core/Math/Memory.hpp"

#include "Core/Result.hpp"
#include "Core/Span.hpp"

#include <cstring>

namespace shmit
{
namespace data
{

/**!
 * @brief Copies the entirety of an object's footprint from a byte buffer to an instance of the object
 *
 * // TODO mention padding
 *
 * @tparam T Decoded object type
 * @param[in] buffer Data source
 * @param[inout] offset_bits Minimum offset, in bits, from beginning of the source that decoding may start from.
 * Updated to include the size, in bits, of the decoded space on success.
 * @param[out] obj Decoding destination, reference to object instance
 * @retval BinaryResult::kSuccessCode if successful
 * @retval BinaryResult::kFailureCode otherwise
 */
template<typename T>
BinaryResult decode(Span<uint8_t const> buffer, size_t& offset_bits, T& obj) noexcept
{
    // Determine byte offset, if nonzero must move to next boundary
    size_t const kDataStartByteOffset {math::bytes_to_contain(offset_bits)};

    // Guard against attempts at underflowing the buffer
    constexpr static size_t kDataFootprintBytes {footprint_size_bytes_v<T>};
    if ((kDataStartByteOffset + kDataFootprintBytes) > buffer.size())
        return BinaryResult::Failure();

    // Perform bytewise copy
    uint8_t*       dest {reinterpret_cast<uint8_t*>(&obj)};
    uint8_t const* src {&buffer[kDataStartByteOffset]};
    static_cast<void>(std::memcpy(dest, src, kDataFootprintBytes));

    // Record the number of bits decoded, including padding
    offset_bits = math::bits_to_contain(kDataStartByteOffset) + footprint_size_bits_v<T>;
    return BinaryResult::Success();
}

} // namespace data
} // namespace shmit