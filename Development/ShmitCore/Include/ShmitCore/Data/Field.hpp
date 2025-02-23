#pragma once

#include "Decode.hpp"
#include "Encode.hpp"
#include "Footprint.hpp"
#include "Primitives.hpp"
#include "_Detail/Field.hpp"

#include "ShmitCore/Math/Memory.hpp"
#include "ShmitCore/Result.hpp"
#include "ShmitCore/Span.hpp"

#include <type_traits>
#include <utility>

namespace shmit
{
namespace data
{

/**!
 * @brief Value storage for mutable data within an organized structure of memory. Padding is provided with this
 * data so that space for the value begins and ends on a byte boundary.
 *
 * @note Field is a representation of encoded data and not the manifestation of it. Data stored in this struct will be
 * aligned in memory and freely accessible/modifiable. Such is not the case for the products of procedures which consume
 * Fields, do not attempt to access data within these results as they may be altered or no longer memory aligned.
 *
 * @warning Always pay attention to the side effects of procedures that consume Fields, the stored value may or may
 * not be destroyed during processing.
 *
 * @tparam T Value type of the Field, must be a pointer or arithmetic type. CV and reference qualifiers are removed.
 */
template<typename T>
struct Field
{
    /// @brief Stored value type
    using value_type = std::decay_t<T>;

    static_assert(std::is_arithmetic<value_type>::value || std::is_pointer<value_type>::value, "`T` must be an "
                                                                                               "arithmetic type or "
                                                                                               "pointer");

    /// @brief Size of the field in bits
    constexpr static size_t kSizeBits {footprint_size_bits_v<T>};

    /// @brief Stored value
    value_type value;
};

/**!
 * @brief Value storage for a mutable section within an organized structure of memory. BitFields treat alignment
 * requirements more like recommendations and simply don't believe in personal space. Placing them in a group encourages
 * them to snuggle up to each other, disregarding any padding that their stored value type might normally enforce. This
 * enables sub-byte placement of data; for instance, one may fit up to 8 single-bit bitfields (commonly referred to as a
 * Bit, let's not be silly) within 1 byte.
 *
 * @note BitField is a representation of encoded data and not the manifestation of it. Data stored in this struct will
 * be aligned in memory and freely accessible/modifiable. Such is not the case for the products of procedures which
 * consume BitFields, do not attempt to access data within these results as they may be altered or no longer memory
 * aligned.
 *
 * @warning Always pay attention to the side effects of procedures that consume BitFields, the stored value may or
 * may not be destroyed during processing.
 *
 * @tparam SizeBitsV Size of the stored value in bits
 */
template<size_t SizeBitsV>
struct BitField
{
    /// @brief Stored value type, the smallest unsigned type that will fit SizeBitsV
    using value_type = smallest_unsigned_t<SizeBitsV>;

    /// @brief Size of the stored value in bits
    constexpr static size_t kSizeBits {SizeBitsV};

    /// @brief Stored value
    value_type value;
};

/// @brief Unit bitfield specialization. The stored value type is bool.
template<>
struct BitField<1>
{
    /// @brief Single bits can be `1`/`0`, `ON`/`OFF`, `true`/`false`, etc.
    using value_type = bool;

    /// @brief Size is 1 bit
    constexpr static size_t kSizeBits {1U};

    /// @brief Stored value
    value_type value;
};

/// @brief Alias for unit mutable bitfield
using Bit = BitField<1U>;

/**!
 * @brief Value storage for a reserved section within an organized structure of memory. ConstBitField subscribes to
 * the same same alternative lifestyle as a regular BitField except that the stored value may not be modified once set.
 *
 * @note ConstBitField is a representation of encoded data and not the manifestation of it. Data stored in this struct
 * will be aligned in memory and freely accessible (but not modifiable). Such is not the case for the products of
 * procedures which consume ConstBitFields, do not attempt to access data within these results as they may be altered
 * or no longer memory aligned.
 *
 * @warning Always pay attention to the side effects of procedures that consume ConstBitFields, the stored value may
 * or may not be destroyed during processing.
 *
 * @tparam SizeBitsV Size of the stored value in bits
 */
template<size_t SizeBitsV>
struct ConstBitField
{
    /// @brief Stored value type, the smallest const unsigned type that will fit `SizeBitsV`
    using value_type = std::add_const_t<typename BitField<SizeBitsV>::value_type>;

    /// @brief Size of the stored value in bits
    constexpr static size_t kSizeBits {SizeBitsV};

    /// @brief Stored value
    value_type value;
};

/// @brief Alias for unit reserved bitfield
using ConstBit = ConstBitField<1U>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace metafunction declarations              ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Converts a type to Field, wrapping the type. Fields are returned as-is.
 *
 * @tparam T Type to potentially wrap with Field
 */
template<typename T>
struct to_field;

/**
 * @brief Unwraps a Field, BitField`, or ConstBitField. Non-wrapped types are returned without cv or reference qualifiers.
 *
 * @tparam T Type to potentially unwrap
 */
template<typename T>
struct to_data;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace function declarations              ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**!
 * @brief Copies the entirety of a Field's footprint from an instance of the Field to a byte buffer
 *
 * @note Padding is provided so that space for the encoded data begins and ends on a byte boundary
 *
 * @tparam T Value type of the Field, must be a pointer or arithmetic type. CV and reference qualifiers are removed.
 * @param[in] field Field to encode
 * @param[in] buffer Encoding destination
 * @param[inout] offset_bits Minimum offset, in bits, from beginning of the destination that encoding may start from.
 * Updated to include the size, in bits, of the encoded space on success.
 * @retval BinaryResult::kSuccessCode if successful
 * @retval BinaryResult::kFailureCode otherwise
 */
template<typename T>
BinaryResult encode(Field<T> const& field, Span<uint8_t> buffer, size_t& offset_bits) noexcept;

/**!
 * @brief Copies the entirety of a BitField's footprint from an instance of the BitField to a byte buffer
 *
 * @tparam SizeBitsV Size of the stored value in bits
 * @param[in] bitfield BitField to encode
 * @param[in] buffer Encoding destination
 * @param[inout] offset_bits Minimum offset, in bits, from beginning of the destination that encoding may start from.
 * Updated to include the size, in bits, of the encoded space on success.
 * @retval BinaryResult::kSuccessCode if successful
 * @retval BinaryResult::kFailureCode otherwise
 */
template<size_t SizeBitsV>
BinaryResult encode(BitField<SizeBitsV> const& bitfield, Span<uint8_t> buffer, size_t& offset_bits) noexcept;

/**!
 * @brief Copies the entirety of a ConstBitField's footprint from an instance of the ConstBitField to a byte buffer
 *
 * @tparam SizeBitsV Size of the stored value in bits
 * @param[in] bitfield ConstBitField to encode
 * @param[in] buffer Encoding destination
 * @param[inout] offset_bits Minimum offset, in bits, from beginning of the destination that encoding may start from.
 * Updated to include the size, in bits, of the encoded space on success.
 * @retval BinaryResult::kSuccessCode if successful
 * @retval BinaryResult::kFailureCode otherwise
 */
template<size_t SizeBitsV>
BinaryResult encode(ConstBitField<SizeBitsV> const& bitfield, Span<uint8_t> buffer, size_t& offset_bits) noexcept;

/**!
 * @brief Copies the entirety of a Field's footprint from a byte buffer to an instance of the Field
 *
 * // TODO mention padding
 *
 * @tparam T Value type of the Field, must be a pointer or arithmetic type. CV and reference qualifiers are removed.
 * @param[in] buffer Data source
 * @param[inout] offset_bits Minimum offset, in bits, from beginning of the source that decoding may start from.
 * Updated to include the size, in bits, of the decoded space on success.
 * @param[out] obj Decoding destination, reference to Field instance
 * @retval BinaryResult::kSuccessCode if successful
 * @retval BinaryResult::kFailureCode otherwise
 */
template<typename T>
BinaryResult decode(Span<uint8_t const> buffer, size_t& offset_bits, Field<T>& field) noexcept;

/**!
 * @brief Copies the entirety of a BitField's footprint from a byte buffer to an instance of the BitField
 *
 * // TODO mention padding
 *
 * @tparam SizeBitsV Size of the stored value in bits
 * @param[in] buffer Data source
 * @param[inout] offset_bits Minimum offset, in bits, from beginning of the source that decoding may start from.
 * Updated to include the size, in bits, of the decoded space on success.
 * @param[out] obj Decoding destination, reference to BitField instance
 * @retval BinaryResult::kSuccessCode if successful
 * @retval BinaryResult::kFailureCode otherwise
 */
template<size_t SizeBitsV>
BinaryResult decode(Span<uint8_t const> buffer, size_t& offset_bits, BitField<SizeBitsV>& bitfield) noexcept;

/**!
 * @brief Copies the entirety of a ConstBitField's footprint from a byte buffer to an instance of the BitField
 *
 * // TODO mention padding
 *
 * @tparam SizeBitsV Size of the stored value in bits
 * @param[in] buffer Data source
 * @param[inout] offset_bits Minimum offset, in bits, from beginning of the source that decoding may start from.
 * Updated to include the size, in bits, of the decoded space on success.
 * @param[out] obj Decoding destination, reference to ConstBitField instance
 * @retval BinaryResult::kSuccessCode if successful
 * @retval BinaryResult::kFailureCode otherwise
 */
template<size_t SizeBitsV>
BinaryResult decode(Span<uint8_t const> buffer, size_t& offset_bits, ConstBitField<SizeBitsV>& bitfield) noexcept;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace metafunction definitions in alphabetical order             ////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct to_data
{
    using type = std::decay_t<T>;
};

template<typename T>
struct to_data<Field<T>>
{
    using type = typename Field<T>::value_type;
};

template<size_t SizeBitsV>
struct to_data<BitField<SizeBitsV>>
{
    using type = typename BitField<SizeBitsV>::value_type;
};

template<size_t SizeBitsV>
struct to_data<ConstBitField<SizeBitsV>>
{
    using type = typename ConstBitField<SizeBitsV>::value_type;
};

/**!
 * @brief Convenience alias to access the returned type of to_data
 *
 * @tparam T Type to potentially unwrap
 */
template<typename T>
using to_data_t = typename to_data<T>::type;

template<typename T>
struct to_field
{
    using type = Field<T>;
};

template<typename T>
struct to_field<Field<T>>
{
    using type = Field<T>;
};

template<size_t SizeBitsV>
struct to_field<BitField<SizeBitsV>>
{
    using type = BitField<SizeBitsV>;
};

template<size_t SizeBitsV>
struct to_field<ConstBitField<SizeBitsV>>
{
    using type = ConstBitField<SizeBitsV>;
};

/**!
 * @brief Convenience alias to access the returned type of to_field
 *
 * @tparam T Type to potentially wrap with Field
 */
template<typename T>
using to_field_t = typename to_field<T>::type;

/**!
 * @brief Field specialization of footprint_size_bits
 *
 * @tparam T Value type of the Field, must be a pointer or arithmetic type. CV and reference qualifiers are removed.
 */
template<typename T>
struct footprint_size_bits<Field<T>>
{
    constexpr static size_t value {Field<T>::kSizeBits};
};

/**!
 * @brief BitField specialization of footprint_size_bits
 *
 * @tparam SizeBitsV Size of the stored value in bits
 */
template<size_t SizeBitsV>
struct footprint_size_bits<BitField<SizeBitsV>>
{
    constexpr static size_t value {BitField<SizeBitsV>::kSizeBits};
};

/**!
 * @brief ConstBitField specialization of footprint_size_bits
 *
 * @tparam SizeBitsV Size of the stored value in bits
 */
template<size_t SizeBitsV>
struct footprint_size_bits<ConstBitField<SizeBitsV>>
{
    constexpr static size_t value {ConstBitField<SizeBitsV>::kSizeBits};
};

/**!
 * @brief Field specialization of footprint_size_bytes
 *
 * @tparam T Value type of the Field, must be a pointer or arithmetic type. CV and reference qualifiers are removed.
 */
template<typename T>
struct footprint_size_bytes<Field<T>>
{
    constexpr static size_t value {math::bytes_to_contain(Field<T>::kSizeBits)};
};

/**!
 * @brief BitField specialization of footprint_size_bytes
 *
 * @tparam SizeBitsV Size of the stored value in bits
 */
template<size_t SizeBitsV>
struct footprint_size_bytes<BitField<SizeBitsV>>
{
    constexpr static size_t value {math::bytes_to_contain(BitField<SizeBitsV>::kSizeBits)};
};

/**!
 * @brief ConstBitField specialization of footprint_size_bytes
 *
 * @tparam SizeBitsV Size of the stored value in bits
 */
template<size_t SizeBitsV>
struct footprint_size_bytes<ConstBitField<SizeBitsV>>
{
    constexpr static size_t value {math::bytes_to_contain(ConstBitField<SizeBitsV>::kSizeBits)};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace function definitions in alphabetical order             ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
BinaryResult encode(Field<T> const& field, Span<uint8_t> buffer, size_t& offset_bits) noexcept
{
    // Pass through to value_type specialized encoding
    return encode(field.value, buffer, offset_bits);
}

template<size_t SizeBitsV>
BinaryResult encode(BitField<SizeBitsV> const& bitfield, Span<uint8_t> buffer, size_t& offset_bits) noexcept
{
    // Guard against attempts at overflowing the buffer
    size_t const kBufferSizeBits {math::bits_to_contain(buffer.size())};
    if ((offset_bits + BitField<SizeBitsV>::kSizeBits) > kBufferSizeBits)
        return BinaryResult::Failure();

    // Perform encoding
    _detail::encode_bits(buffer.data(), reinterpret_cast<uint8_t const*>(&bitfield.value), offset_bits,
                         BitField<SizeBitsV>::kSizeBits);

    // Record the number of bits encoded
    offset_bits += BitField<SizeBitsV>::kSizeBits;
    return BinaryResult::Success();
}

template<size_t SizeBitsV>
BinaryResult encode(ConstBitField<SizeBitsV> const& bitfield, Span<uint8_t> buffer, size_t& offset_bits) noexcept
{
    // Guard against attempts at overflowing the buffer
    size_t const kBufferSizeBits {math::bits_to_contain(buffer.size())};
    if ((offset_bits + ConstBitField<SizeBitsV>::kSizeBits) > kBufferSizeBits)
        return BinaryResult::Failure();

    // Perform encoding
    _detail::encode_bits(buffer.data(), reinterpret_cast<uint8_t const*>(&bitfield.value), offset_bits,
                         ConstBitField<SizeBitsV>::kSizeBits);

    // Record the number of bits encoded
    offset_bits += ConstBitField<SizeBitsV>::kSizeBits;
    return BinaryResult::Success();
}

template<typename T>
BinaryResult decode(Span<uint8_t const> buffer, size_t& offset_bits, Field<T>& field) noexcept
{
    // Pass through to value_type specialized decoding
    return decode(buffer, offset_bits, field.value);
}

template<size_t SizeBitsV>
BinaryResult decode(Span<uint8_t const> buffer, size_t& offset_bits, BitField<SizeBitsV>& bitfield) noexcept
{
    // Guard against attempts at underflowing the buffer
    size_t const kBufferSizeBits {math::bits_to_contain(buffer.size())};
    if ((offset_bits + BitField<SizeBitsV>::kSizeBits) > kBufferSizeBits)
        return BinaryResult::Failure();

    // Perform decoding
    _detail::decode_bits(reinterpret_cast<uint8_t*>(&bitfield.value), buffer.data(), offset_bits,
                         BitField<SizeBitsV>::kSizeBits);

    // Record the number of bits decoded
    offset_bits += BitField<SizeBitsV>::kSizeBits;
    return BinaryResult::Success();
}

template<size_t SizeBitsV>
BinaryResult decode(Span<uint8_t const> buffer, size_t& offset_bits, ConstBitField<SizeBitsV>& bitfield) noexcept
{
    // A const value can't be reassigned, so nothing that is decoded can actually be stored in bitfield
    static_cast<void>(buffer);   // Avoid unused warning
    static_cast<void>(bitfield); // Avoid unused warning

    // Do nothing but increase offset_bits by bitfield's size
    offset_bits += ConstBitField<SizeBitsV>::kSizeBits;
    return BinaryResult::Success();
}

} // namespace data
} // namespace shmit