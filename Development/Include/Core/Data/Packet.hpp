#pragma once

#include "Decode.hpp"
#include "Encode.hpp"
#include "Field.hpp"
#include "Footprint.hpp"
#include "_Detail/Packet.hpp"

#include "Core/Math/Memory.hpp"
#include "Core/Result.hpp"
#include "Core/Span.hpp"

#include <tuple>
#include <type_traits>

namespace shmit
{
namespace data
{

/**!
 * @brief Data's final form, Packet contains a collection of value wrappers -- fields -- that comprise an organized
 * structure of memory.
 *
 * @note Packet is a representation of encoded data and not the manifestation of it. Data for all fields stored in this
 * class will be aligned in memory and freely accessible. Such is not the case for the products of procedures which
 * consume Packets, do not attempt to access data within these results as they may be altered or no longer memory
 * aligned.
 *
 * @warning Always pay attention to the side effects of procedures that consume Packets or their constituent fields,
 * stored values may or may not be destroyed during processing.
 *
 * @tparam FieldT Type parameter list representing the values to be held as fields by the Packet. Fields are
 * stored in the order they are presented and are accessible through their positional index, starting at 0. Wrapped
 * types such as Field, BitField, and ConstBitField are stored as-is while unwrapped types will become wrapped by Field.
 */
template<typename... FieldT>
class Packet : public _detail::PacketBase
{
public:
    /// @brief Alias to sanitized type identification
    using type = Packet<to_field_t<FieldT>...>;

    /// @brief Alias to fields container type
    using Fields = std::tuple<to_field_t<FieldT>...>;

    /**!
     * @brief Returns type information of the value stored by a field
     *
     * @tparam IndexV Field index
     */
    template<size_t IndexV>
    struct value_info
    {
        using value_type      = typename _detail::fetch_packet_field_type_info<IndexV, type>::value_type;
        using value_reference = typename _detail::fetch_packet_field_type_info<IndexV, type>::value_reference;
        using const_value_reference = typename _detail::fetch_packet_field_type_info<IndexV, type>::const_value_reference;
    };

    /**!
     * @brief Convenience alias to access value_type from the value_info of a field
     *
     * @tparam IndexV Field index
     */
    template<size_t IndexV>
    using value_type_t = typename value_info<IndexV>::value_type;

    /**!
     * @brief Convenience alias to access value_reference from the value_info of a field
     *
     * @tparam IndexV Field index
     */
    template<size_t IndexV>
    using value_reference_t = typename value_info<IndexV>::value_reference;

    /**!
     * @brief Convenience alias to access const_value_reference from the value_info of a field
     *
     * @tparam IndexV Field index
     */
    template<size_t IndexV>
    using const_value_reference_t = typename value_info<IndexV>::const_value_reference;

    /// @brief Total number of fields held by the Packet
    constexpr static size_t kNumFields {std::tuple_size<Fields>::value};

    /// @brief Total size in bits required to contain the memory structure defined by Fields including padding between elements
    constexpr static size_t kSizeBits {_detail::packet_size_bits<type>::value};

    /// @brief Total size in bytes required to contain the memory structure defined by Fields including padding between elements
    constexpr static size_t kSizeBytes {math::bytes_to_contain(kSizeBits)};

    /**!
     * @brief Initializing constructor. Takes arguments for every field.
     *
     * @param[in] args Values to initialize each field, in order
     */
    template<typename... ArgT>
    Packet(ArgT&&... args) noexcept;

    /**!
     * @brief Initializing constructor. Takes another Packet of the same specialization and forwards to the copy constructor.
     *
     * @param[in] initialize Reference to Packet specialization to initialize with
     */
    Packet(Packet& initialize) noexcept;

    // Packet is trivially default, copy, and move constructible... and destructible

    Packet() noexcept                   = default;
    Packet(Packet const& copy) noexcept = default;
    Packet(Packet&& move) noexcept      = default;
    ~Packet() noexcept                  = default;

    // Packet is trivially copy and move assignable

    Packet& operator=(Packet const& copy) noexcept = default;
    Packet& operator=(Packet&& move) noexcept      = default;

    /**!
     * @brief Copies the entirety of a Packet's footprint from an instance's held fields to a byte buffer in the order
     * that they are stored
     *
     * @note Padding is provided so that space for the encoded Packet begins and ends on a byte boundary
     * @note Padding is applied to all Field specializations held by the packet
     * @note Bitpacking is applied to any groups of sequential BitField and/or ConstBitField specializations held by the
     * packet
     *
     * @tparam AltFieldT Type parameter list representing the values to be held as fields by the Packet. Fields are
     * stored in the order they are presented and are accessible through their positional index, starting at 0. Wrapped
     * types such as Field, BitField, and ConstBitField are stored as-is while unwrapped types will be wrapped by Field.
     * @param[in] packet Packet to encode
     * @param[in] buffer Encoding destination
     * @param[inout] offset_bits Minimum offset, in bits, from beginning of the destination that encoding may start
     * from. Updated to the tail byte boundary of the object's encoded footprint on success.
     * @retval BinaryResult::kSuccessCode if successful
     * @retval BinaryResult::kFailureCode otherwise
     */
    template<typename... AltFieldT>
    friend BinaryResult encode(Packet<AltFieldT...> const& packet, Span<uint8_t> buffer, size_t& offset_bits) noexcept;

    /**!
     * @brief Copies the entirety of a Packet's footprint from a byte buffer to an instance's held fields in the order
     * that they are stored
     *
     * // TODO mention padding
     *
     * @tparam AltFieldT Type parameter list representing the values to be held as fields by the Packet. Fields are
     * stored in the order they are presented and are accessible through their positional index, starting at 0. Wrapped
     * types such as Field, BitField, and ConstBitField are stored as-is while unwrapped types will be wrapped by Field.
     * @param[in] buffer Data source
     * @param[inout] offset_bits Minimum offset, in bits, from beginning of the source that decoding may start from.
     * Updated to include the size, in bits, of the decoded space on success.
     * @param[out] packet Decoding destination, reference to Packet instance
     * @retval BinaryResult::kSuccessCode if successful
     * @retval BinaryResult::kFailureCode otherwise
     */
    template<typename... AltFieldT>
    friend BinaryResult decode(Span<uint8_t const> buffer, size_t& offset_bits, Packet<AltFieldT...>& packet) noexcept;

    /**!
     * @brief Fetch a value reference from a field held by a packet
     *
     * @tparam IndexV Field index to access
     * @tparam AltFieldT Type parameter list representing the values to be held as fields by the Packet. Fields are
     * stored in the order they are presented and are accessible through their positional index, starting at 0. Wrapped
     * types such as Field, BitField, and ConstBitField are stored as-is while unwrapped types will be wrapped by Field.
     * @param[in] packet Packet to fetch field from
     * @return Reference to stored value
     */
    template<size_t IndexV, typename... AltFieldT>
    friend typename Packet<AltFieldT...>::value_reference_t<IndexV> packet_field_value(Packet<AltFieldT...>& packet) noexcept;

    /**!
     * @brief Fetch a const value reference from a field held by a packet
     *
     * @tparam IndexV Field index to access
     * @tparam AltFieldT Type parameter list representing the values to be held as fields by the Packet. Fields are
     * stored in the order they are presented and are accessible through their positional index, starting at 0. Wrapped
     * types such as Field, BitField, and ConstBitField are stored as-is while unwrapped types will be wrapped by Field.
     * @param[in] packet Packet to fetch field from
     * @return Const reference to stored value
     */
    template<size_t IndexV, typename... AltFieldT>
    friend typename Packet<AltFieldT...>::const_value_reference_t<IndexV>
        packet_field_value(Packet<AltFieldT...> const& packet) noexcept;

private:
    /// @brief Container of fields
    Fields m_fields;
};

/**!
 * @brief Convenience alias to access sanitized type identification of a Packet
 *
 * @tparam FieldT Type parameter list representing the values to be held as fields by the Packet. Fields are stored
 * in the order they are presented and are accessible through their index, starting at 0. Wrapped types such as Field,
 * BitField, and ConstBitField are stored as-is while unwrapped types will be wrapped by Field.
 */
template<typename... FieldT>
using packet_t = typename Packet<FieldT...>::type;

/**!
 * @brief Field specialization for a nested packet
 *
 * @note Field is a representation of encoded data and not the manifestation of it. Data stored in this struct will be
 * aligned in memory and freely accessible/modifiable. Such is not the case for the products of procedures which consume
 * Fields, do not attempt to access data within these results as they may be altered or no longer memory aligned.
 *
 * @warning Always pay attention to the side effects of procedures that consume Fields, the stored value may or may
 * not be destroyed during processing.
 *
 * @tparam FieldT Type parameter list representing the values to be held as fields by the Packet. Fields are stored
 * in the order they are presented and are accessible through their index, starting at 0. Wrapped types such as Field,
 * BitField, and ConstBitField are stored as-is while unwrapped types will be wrapped by Field.
 */
template<typename... FieldT>
struct Field<Packet<FieldT...>>
{
    /// @brief Stored value type is Packet specialization
    using value_type = typename Packet<FieldT...>::type;

    /// @brief Size of nested packet in bits
    constexpr static size_t kSizeBits {value_type::kSizeBits};

    /// @brief Stored value
    value_type value;
};

/**!
 * @brief Packet specialization of footprint_size_bits
 *
 * @tparam FieldT Type parameter list representing the values to be held as fields by the Packet. Fields are stored
 * in the order they are presented and are accessible through their index, starting at 0. Wrapped types such as Field,
 * BitField, and ConstBitField are stored as-is while unwrapped types will be wrapped by Field.
 */
template<typename... FieldT>
struct footprint_size_bits<Packet<FieldT...>>
{
    constexpr static size_t value {Packet<FieldT...>::kSizeBits};
};

/**!
 * @brief Packet specialization of footprint_size_bytes
 *
 * @tparam FieldT Type parameter list representing the values to be held as fields by the Packet. Fields are stored
 * in the order they are presented and are accessible through their index, starting at 0. Wrapped types such as Field,
 * BitField, and ConstBitField are stored as-is while unwrapped types will be wrapped by Field.
 */
template<typename... FieldT>
struct footprint_size_bytes<Packet<FieldT...>>
{
    constexpr static size_t value {Packet<FieldT...>::kSizeBytes};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace metafunction definitions               ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**!
 * @brief Checks if type is a Packet specialization
 *
 * @tparam T Potential Packet type
 */
template<typename T>
using is_packet = _detail::is_packet<T>;

/**!
 * @brief Convenience alias to access the returned type of is_packet
 *
 * @tparam T Potential Packet type
 */
template<typename T>
using is_packet_t = typename is_packet<T>::type;

/**!
 * @brief Convenience alias to access the returned value of is_packet
 *
 * @tparam T Potential Packet type
 */
template<typename T>
constexpr static bool is_packet_v {is_packet<T>::value};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Packet constructor definitions               ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename... FieldT>
template<typename... ArgT>
Packet<FieldT...>::Packet(ArgT&&... args) noexcept : m_fields {to_field_t<FieldT> {args}...}
{
}

template<typename... FieldT>
Packet<FieldT...>::Packet(Packet<FieldT...>& initialize) noexcept : Packet {const_cast<const Packet&>(initialize)}
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace function definitions in alphabetical order             ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename... FieldT>
BinaryResult encode(Packet<FieldT...> const& packet, Span<uint8_t> buffer, size_t& offset_bits) noexcept
{
    using Fields = typename Packet<FieldT...>::Fields;

    // Determine start offset, must be on byte boundary
    size_t const kDataStartOffsetBytes {math::bytes_to_contain(offset_bits)};
    size_t       encode_offset_bits {math::bits_to_contain(kDataStartOffsetBytes)};

    // Guard against attempts at overflowing the buffer
    if ((kDataStartOffsetBytes + Packet<FieldT...>::kSizeBytes) > buffer.size())
        return BinaryResult::Failure();

    // Perform recursive encoding of Packet fields
    BinaryResult result {_detail::encode_packet_fields_recursive<0U, Packet<FieldT...>::kNumFields, Packet<FieldT...>>::Do(
        buffer, encode_offset_bits, packet.m_fields)};

    // Apply padding to the offset result, if encoding was successful, so that it ends on a byte boundary
    if (result.IsSuccess())
        offset_bits = math::next_boundary_bit_pos(encode_offset_bits);

    return result;
}

template<typename... FieldT>
BinaryResult decode(Span<uint8_t const> buffer, size_t& offset_bits, Packet<FieldT...>& packet) noexcept
{
    using Fields = typename Packet<FieldT...>::Fields;

    // Determine start offset, must be on byte boundary
    size_t const kDataStartOffsetBytes {math::bytes_to_contain(offset_bits)};
    size_t       encode_offset_bits {math::bits_to_contain(kDataStartOffsetBytes)};

    // Guard against attempts at underflowing the buffer
    if ((kDataStartOffsetBytes + Packet<FieldT...>::kSizeBytes) > buffer.size())
        return BinaryResult::Failure();

    // Perform recursive decoding of Packet fields
    BinaryResult result {_detail::decode_packet_fields_recursive<0U, Packet<FieldT...>::kNumFields, Packet<FieldT...>>::Do(
        buffer, encode_offset_bits, packet.m_fields)};

    // Apply padding to the offset result, if decoding was successful, so that it ends on a byte boundary
    if (result.IsSuccess())
        offset_bits = math::next_boundary_bit_pos(encode_offset_bits);

    return result;
}

template<size_t IndexV, typename... FieldT>
typename Packet<FieldT...>::value_reference_t<IndexV> packet_field_value(Packet<FieldT...>& packet) noexcept
{
    using Field = typename _detail::fetch_packet_field_type_info<IndexV, Packet<FieldT...>>::type;
    Field& field {std::get<IndexV>(packet.m_fields)};
    return field.value;
}

template<size_t IndexV, typename... FieldT>
typename Packet<FieldT...>::const_value_reference_t<IndexV> packet_field_value(Packet<FieldT...> const& packet) noexcept
{
    using Field = typename _detail::fetch_packet_field_type_info<IndexV, Packet<FieldT...>>::type;
    Field const& field {std::get<IndexV>(packet.m_fields)};
    return field.value;
}

} // namespace data
} // namespace shmit