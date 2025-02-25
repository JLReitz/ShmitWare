#pragma once

#include "Core/Math/Memory.hpp"
#include "Core/StdTypes.hpp"

#include <algorithm>
#include <array>

namespace shmit
{
namespace data
{
namespace _detail
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace function definitions in alphabetical order             ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**!
 * @brief Copy a number of bits from a byte-aligned source to a destination memory address and bit offset
 *
 * @param[in] dest Destination address
 * @param[in] src Source address
 * @param[in] offset_bits Start bit offset for copying to the destination
 * @param[in] size_bits Number of bits to copy
 */
static void encode_bits(uint8_t* dest, uint8_t const* src, size_t offset_bits, size_t size_bits)
{
    constexpr size_t                                  kModulo8Mask {0x7};
    constexpr size_t                                  kBitsPerByte {math::bits_to_contain(1U)};
    constexpr std::array<uint8_t, (kBitsPerByte + 1)> kBitMasks {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};

    // Determine byte and relative positional bit offsets
    size_t offset_bytes {math::bytes_to_contain(offset_bits)};
    offset_bits &= kModulo8Mask; // Cap bit offset at 8, keep remainder

    // bytes_to_contain() will ceiling the returned value because at least 1 byte is required to hold any non-zero
    // amount of bits. If not beginning at a byte boundary, or within the first byte, roll the byte offset back to the
    // previous, partially-populated position.
    if ((offset_bits > 0U) && (offset_bytes > 0U))
        offset_bytes -= 1;

    // Move destination address up by the byte offset
    dest = dest + offset_bytes;

    // Encode data
    size_t const kStartByteBitsAvailable {kBitsPerByte - offset_bits};
    while (size_bits > 0U)
    {
        // Encode the bits that fit from the offset position to the next byte boundary
        size_t const kFrontBitsThatFit {std::min(kStartByteBitsAvailable, size_bits)};
        *dest++ |= ((*src & kBitMasks[kFrontBitsThatFit]) << offset_bits);
        size_bits -= kFrontBitsThatFit;
        if ((offset_bits > 0U) && (size_bits > 0U))
        {
            // Value wraps over byte boundary, clean up leftovers from the current source byte
            // Mask the resultant tail so that residue from the copy is not left over.
            size_t const kLeftoverBits {std::min(offset_bits, size_bits)};
            *dest = ((*src >> kFrontBitsThatFit) & kBitMasks[kLeftoverBits]);
            size_bits -= kLeftoverBits;
        }

        src++;
    }
}

/**!
 * @brief Copy a number of bits from a source memory address and bit offset in to a byte-aligned destination
 *
 * @param dest Destination address
 * @param src Source address
 * @param offset_bits Start bit offset for copying from the source
 * @param size_bits Number of bits to copy
 */
static void decode_bits(uint8_t* dest, uint8_t const* src, size_t offset_bits, size_t size_bits)
{
    constexpr size_t                                  kModulo8Mask {0x7};
    constexpr size_t                                  kBitsPerByte {math::bits_to_contain(1U)};
    constexpr std::array<uint8_t, (kBitsPerByte + 1)> kBitMasks {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};

    // Determine byte and relative positional bit offsets
    size_t offset_bytes {math::bytes_to_contain(offset_bits)};
    offset_bits &= kModulo8Mask; // Cap bit offset at 8, keep remainder

    // bytes_to_contain() will ceiling the returned value because at least 1 byte is required to hold any non-zero
    // amount of bits. If not beginning at a byte boundary, or within the first byte, roll the byte offset back to the
    // previous, partially-populated position.
    if ((offset_bits > 0U) && (offset_bytes > 0U))
        offset_bytes -= 1;

    // Move source address up by the byte offset
    src = src + offset_bytes;

    // Decode data
    size_t const kStartByteBitsAvailable {kBitsPerByte - offset_bits};
    while (size_bits > 0U)
    {
        // Encode the bits that fit from the offset position to the next byte boundary
        size_t const kFrontBitsThatFit {std::min(kStartByteBitsAvailable, size_bits)};
        *dest = ((*src++ >> offset_bits) & kBitMasks[kFrontBitsThatFit]);
        size_bits -= kFrontBitsThatFit;
        if ((offset_bits > 0U) && (size_bits > 0U))
        {
            // Value wraps over byte boundary, copy over tail for current destination byte
            size_t const kLeftoverBits {std::min(offset_bits, size_bits)};
            *dest |= ((*src & kBitMasks[kLeftoverBits]) << kFrontBitsThatFit);
            size_bits -= kLeftoverBits;
        }

        dest++;
    }
}

} // namespace _detail
} // namespace data
} // namespace shmit