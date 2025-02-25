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
 * @brief Copies the entirety of an object's footprint from an instance of the object to a byte buffer
 *
 * @note Padding is provided so that space for the encoded data begins and ends on a byte boundary
 *
 * @tparam T Object type
 * @param[in] obj Object to encode
 * @param[in] buffer Encoding destination
 * @param[inout] offset_bits Minimum offset, in bits, from beginning of the destination that encoding may start from.
 * Updated to the tail byte boundary of the object's encoded footprint on success.
 * @retval BinaryResult::kSuccessCode if successful
 * @retval BinaryResult::kFailureCode otherwise
 */
template<typename T>
BinaryResult encode(T const& obj, Span<uint8_t> buffer, size_t& offset_bits) noexcept
{
    // Determine byte offset, if nonzero must move to next boundary
    size_t const kDataStartByteOffset {math::bytes_to_contain(offset_bits)};

    // Guard against attempts at overflowing the buffer
    constexpr static size_t kDataFootprintBytes {footprint_size_bytes_v<T>};
    if ((kDataStartByteOffset + kDataFootprintBytes) > buffer.size())
        return BinaryResult::Failure();

    // Perform bytewise copy
    uint8_t const* src {reinterpret_cast<uint8_t const*>(&obj)};
    uint8_t*       dest {&buffer[kDataStartByteOffset]};
    static_cast<void>(std::memcpy(dest, src, kDataFootprintBytes));

    // Record the number of bits encoded, including padding
    offset_bits = math::bits_to_contain(kDataStartByteOffset) + footprint_size_bits_v<T>;
    return BinaryResult::Success();
}

} // namespace data
} // namespace shmit