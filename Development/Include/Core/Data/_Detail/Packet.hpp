#pragma once

#include "Core/Data/Decode.hpp"
#include "Core/Data/Field.hpp"
#include "Core/Data/Footprint.hpp"
#include "Core/Math/Memory.hpp"
#include "Core/StdTypes.hpp"

#include <tuple>
#include <type_traits>

namespace shmit
{
namespace data
{
namespace _detail
{

/// @brief Object for all specializations of Packet to inherit from, providing a common identity
struct PacketBase
{
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace function definitions in alphabetical order             ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**!
 * @brief Adds the size of a Field's stored value type in bits
 *
 * @tparam T Value type of the Field, cv and reference qualifiers are removed
 * @param[in] aggregate Value to add the Field's size to
 * @param[in] field Field instance
 * @retval Aggregated size of the Field
 */
template<typename T>
constexpr static size_t add_field_size_bits(size_t aggregate, Field<T> field)
{
    size_t const kAggregateBytes {math::bytes_to_contain(aggregate)};
    return math::bits_to_contain(kAggregateBytes) + Field<T>::kSizeBits;
}

/**!
 * @brief Adds the size of a BitField's stored value in bits
 *
 * @tparam SizeBitsV Size of the stored value in bits
 * @param[in] aggregate Value to add the Field's size to
 * @param[in] field Field instance
 * @retval Aggregated size of the Field
 */
template<size_t SizeBitsV>
constexpr static size_t add_field_size_bits(size_t aggregate, BitField<SizeBitsV> field)
{
    return aggregate + BitField<SizeBitsV>::kSizeBits;
}

/**!
 * @brief Adds the size of a ConstBitField's stored value type in bits
 *
 * @tparam SizeBitsV Size of the stored value in bits
 * @param[in] aggregate Value to add the Field's size to
 * @param[in] field Field instance
 * @retval Aggregated size of the Field
 */
template<size_t SizeBitsV>
constexpr static size_t add_field_size_bits(size_t aggregate, ConstBitField<SizeBitsV> field)
{
    return aggregate + ConstBitField<SizeBitsV>::kSizeBits;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Namespace metafunction definitions               ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**!
 * @brief Checks if type is a Packet specialization
 *
 * @tparam T Potential Packet type
 */
template<typename T>
using is_packet = typename std::is_base_of<_detail::PacketBase, T>::type;

/**!
 * @brief Accumulates the size of fields held by a packet
 *
 * @tparam IndexV Current field position
 * @tparam EndV Endpoint for recursion
 * @tparam PacketT Packet specialization
 */
template<size_t IndexV, size_t EndV, typename PacketT>
struct accumulate_packet_field_size_bits_recursive
{
    static_assert(is_packet<PacketT>::value, "`PacketT` must be a `shmit::data::Packet` specialization");
    static_assert((IndexV < PacketT::kNumFields), "`IndexV` must be within `PacketT` field count");
    static_assert(IndexV < EndV, "`IndexV` must be within `EndV` bounds");

private:
    using FieldPack = typename PacketT::Fields;
    using Field     = typename std::tuple_element<IndexV, FieldPack>::type;

public:
    /**
     * @brief Accumulate size of current field position
     *
     * @param[in] aggregate Starting value
     * @retval Accumulated value
     */
    constexpr static size_t Do(size_t aggregate)
    {
        aggregate = add_field_size_bits(aggregate, Field {});
        return accumulate_packet_field_size_bits_recursive<IndexV + 1, EndV, PacketT>::Do(aggregate);
    }
};

/**!
 * @brief Base case for recursive accumulation of Packet field size; `IndexV == EndV`
 *
 * @tparam IndexV Index of field to accumulate size for, reached endpoint
 * @tparam PacketT Packet specialization
 */
template<size_t IndexV, typename PacketT>
struct accumulate_packet_field_size_bits_recursive<IndexV, IndexV, PacketT>
{
    /**
     * @brief At end of packet, do nothing
     *
     * @param[in] aggregate Unused
     * @retval aggregate
     */
    constexpr static size_t Do(size_t aggregate)
    {
        return aggregate;
    }
};

/**
 * @brief Sequentially encodes the fields held by a packet in to a byte buffer
 *
 * @tparam IndexV Current field position
 * @tparam EndV Endpoint for recursion
 * @tparam PacketT Packet specialization
 */
template<size_t IndexV, size_t EndV, typename PacketT>
struct encode_packet_fields_recursive
{
    static_assert(is_packet<PacketT>::value, "`PacketT` must be a `shmit::data::Packet` specialization");
    static_assert((EndV <= PacketT::kNumFields), "`EndV` must not be greater than `PacketT` field count");
    static_assert(IndexV < EndV, "`IndexV` must be within `EndV` bounds");

private:
    using FieldPack = typename PacketT::Fields;

public:
    /**
     * @brief Encode value at current field position in to byte buffer
     *
     * @param[in] buffer Encoding destination
     * @param[inout] offset_bits Minimum offset, in bits, from beginning of the destination that encoding may start
     * from. Updated to the tail byte boundary of the object's encoded footprint on success.
     * @param[in] data Source packet fields pack (tuple specialization)
     * @retval BinaryResult::kSuccessCode if successful
     * @retval BinaryResult::kFailureCode otherwise
     */
    static BinaryResult Do(Span<uint8_t> buffer, size_t& offset_bits, FieldPack const& data)
    {
        auto&        field {std::get<IndexV>(data)};
        BinaryResult result {encode(field, buffer, offset_bits)};
        if (result.IsFailure())
            return result;

        return encode_packet_fields_recursive<(IndexV + 1U), EndV, PacketT>::Do(buffer, offset_bits, data);
    }
};

/**
 * @brief Base case for recursive encoding of packet fields; `IndexV == EndV`
 *
 * @tparam IndexV Index of field to accumulate size for, reached endpoint
 * @tparam PacketT Packet specialization
 */
template<size_t IndexV, typename PacketT>
struct encode_packet_fields_recursive<IndexV, IndexV, PacketT>
{
private:
    using FieldPack = typename PacketT::Fields;

public:
    /**
     * @brief At end of packet, do nothing
     *
     * @param[in] buffer Encoding destination
     * @param[inout] offset_bits Minimum offset, in bits, from beginning of the destination that encoding may start
     * from. Updated to the tail byte boundary of the object's encoded footprint on success.
     * @param[out] data Source packet fields pack (tuple specialization)
     * @retval BinaryResult::kSuccessCode
     */
    static BinaryResult Do(Span<uint8_t> buffer, size_t& offset_bits, FieldPack const& data)
    {
        static_cast<void>(buffer);      // Avoid unused warning
        static_cast<void>(offset_bits); // Avoid unused warning
        static_cast<void>(data);        // Avoid unused warning
        return BinaryResult::Success();
    }
};

/**
 * @brief Sequentially decodes the fields held by a packet from a byte buffer
 *
 * @tparam IndexV Current field position
 * @tparam EndV Endpoint for recursion
 * @tparam PacketT Packet specialization
 */
template<size_t IndexV, size_t EndV, typename PacketT>
struct decode_packet_fields_recursive
{
    static_assert(is_packet<PacketT>::value, "`PacketT` must be a `shmit::data::Packet` specialization");
    static_assert((EndV <= PacketT::kNumFields), "`EndV` must not be greater than `PacketT` field count");
    static_assert(IndexV < EndV, "`IndexV` must be within `EndV` bounds");

private:
    using FieldPack = typename PacketT::Fields;

public:
    /**
     * @brief Decode value from a byte buffer in to the current field position
     *
     * @param[in] buffer Data source
     * @param[inout] offset_bits Minimum offset, in bits, from beginning of the source that decoding may start from.
     * Updated to include the size, in bits, of the decoded space on success.
     * @param[out] data Decoding destination, Packet fields pack (tuple specialization)
     * @retval BinaryResult::kSuccessCode if successful
     * @retval BinaryResult::kFailureCode otherwise
     */
    static BinaryResult Do(Span<uint8_t const> buffer, size_t& offset_bits, FieldPack& data)
    {
        auto&        field {std::get<IndexV>(data)};
        BinaryResult result {decode(buffer, offset_bits, field)};
        if (result.IsFailure())
            return result;

        return decode_packet_fields_recursive<(IndexV + 1U), EndV, PacketT>::Do(buffer, offset_bits, data);
    }
};

/**
 * @brief Base case for recursive decoding of packet fields; `IndexV == EndV`
 *
 * @tparam IndexV Index of field to accumulate size for, reached endpoint
 * @tparam PacketT Packet specialization
 */
template<size_t IndexV, typename PacketT>
struct decode_packet_fields_recursive<IndexV, IndexV, PacketT>
{
private:
    using FieldPack = typename PacketT::Fields;

public:
    /**
     * @brief At end of packet, do nothing
     *
     * @param[in] buffer Data source
     * @param[inout] offset_bits Minimum offset, in bits, from beginning of the source that decoding may start from.
     * Updated to include the size, in bits, of the decoded space on success.
     * @param[out] packet Decoding destination, reference to Packet instance
     * @retval BinaryResult::kSuccessCode
     */
    static BinaryResult Do(Span<uint8_t const> buffer, size_t& offset_bits, FieldPack& data)
    {
        // At end of packet, do nothing
        static_cast<void>(buffer);      // Avoid unused warning
        static_cast<void>(offset_bits); // Avoid unused warning
        static_cast<void>(data);        // Avoid unused warning
        return BinaryResult::Success();
    }
};

/**!
 * @brief Returns type information on the field stored within a Packet
 *
 * @tparam IndexV Index of field within Packet
 * @tparam PacketT Packet specialization
 */
template<size_t IndexV, typename PacketT>
struct fetch_packet_field_type_info
{
    static_assert(is_packet<PacketT>::value, "`PacketT` must be a `shmit::data::Packet` specialization");
    static_assert((IndexV < PacketT::kNumFields), "`IndexV` must be within `PacketT` field count");

private:
    using FieldPack = typename PacketT::Fields;

public:
    using type           = typename std::tuple_element<IndexV, FieldPack>::type;
    using type_reference = std::add_lvalue_reference_t<type>;

    using value_type            = typename type::value_type;
    using value_reference       = std::add_lvalue_reference_t<value_type>;
    using const_value_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;
};

/**!
 * @brief Returns the size of a packet in bits
 *
 * @tparam PacketT Packet specialization
 */
template<typename PacketT>
struct packet_size_bits
{
    static_assert(is_packet<PacketT>::value, "`PacketT` must be a `shmit::data::Packet` specialization");

private:
    constexpr static size_t kAccumulatedFieldsSizeBits {
        accumulate_packet_field_size_bits_recursive<0U, PacketT::kNumFields, PacketT>::Do(0U)};

public:
    constexpr static size_t value {math::next_boundary_bit_pos(kAccumulatedFieldsSizeBits)};
};

} // namespace _detail
} // namespace data
} // namespace shmit