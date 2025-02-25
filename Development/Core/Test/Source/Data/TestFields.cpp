#include "Help.hpp"

#include <Core/Data/Field.hpp>
#include <Core/Math/Memory.hpp>
#include <Core/Span.hpp>

#include <gtest/gtest.h>

#include <cstring>
#include <type_traits>

using namespace shmit;
using namespace shmit::data;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Field tests             ////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Test that values stored by a Field are stripped of their reference and CV qualifications
 *
 */
TEST(Field, stored_type_decays_qualifications)
{
    using decayed_type = int;

    using const_type          = std::add_const_t<decayed_type>;
    using volatile_type       = std::add_volatile_t<decayed_type>;
    using const_field_test    = Field<const_type>;
    using volatile_field_test = Field<volatile_type>;
    ::testing::StaticAssertTypeEq<decayed_type, typename const_field_test::value_type>();
    ::testing::StaticAssertTypeEq<decayed_type, typename volatile_field_test::value_type>();

    using pointer_type             = std::add_pointer_t<decayed_type>;
    using const_pointer_type       = std::add_const_t<pointer_type>;
    using const_pointer_field_test = Field<const_pointer_type>;
    ::testing::StaticAssertTypeEq<pointer_type, typename const_pointer_field_test::value_type>();

    using lvalue_reference_type = std::add_lvalue_reference_t<decayed_type>;
    using rvalue_reference_type = std::add_rvalue_reference_t<decayed_type>;
    using lvalue_field_test     = Field<lvalue_reference_type>;
    using rvalue_field_test     = Field<rvalue_reference_type>;
    ::testing::StaticAssertTypeEq<decayed_type, typename lvalue_field_test::value_type>();
    ::testing::StaticAssertTypeEq<decayed_type, typename rvalue_field_test::value_type>();

    using const_reference_type       = std::add_const_t<lvalue_reference_type>;
    using const_reference_field_test = Field<const_reference_type>;
    ::testing::StaticAssertTypeEq<decayed_type, typename const_reference_field_test::value_type>();
}

/**
 * Test that values stored by different Fields are sequentially encoded in to a byte buffer
 *
 */
TEST(Field, encode_sequential)
{
    using FirstField  = Field<int>;
    using SecondField = Field<uint16_t>;
    using ThirdField  = Field<float>;

    constexpr size_t kFirstFieldSizeBytes {math::bytes_to_contain(FirstField::kSizeBits)};
    constexpr size_t kSecondFieldSizeBytes {math::bytes_to_contain(SecondField::kSizeBits)};
    constexpr size_t kThirdFieldSizeBytes {math::bytes_to_contain(ThirdField::kSizeBits)};

    typename FirstField::value_type first_value {255};
    FirstField                      first {first_value};

    typename SecondField::value_type second_value {0xA5A5};
    SecondField                      second {second_value};

    typename ThirdField::value_type third_value {3.14};
    ThirdField                      third {third_value};

    constexpr size_t kFieldsSizeBytes {kFirstFieldSizeBytes + kSecondFieldSizeBytes + kThirdFieldSizeBytes};
    uint8_t          bytes[kFieldsSizeBytes];
    Span<uint8_t>    byte_span {bytes, kFieldsSizeBytes};
    size_t           bits_encoded {0U};

    // Clear destination bytes before encoding
    static_cast<void>(std::memset(bytes, 0U, kFieldsSizeBytes));

    // Encode first field, validate that its memory contents match the encoded result
    Span<uint8_t> first_field_encoded_span {byte_span.data(), kFirstFieldSizeBytes};
    Span<uint8_t> first_value_span {reinterpret_cast<uint8_t*>(&first_value), kFirstFieldSizeBytes};

    auto result {encode(first, byte_span, bits_encoded)};
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(byte_spans_match(first_field_encoded_span, first_value_span));

    // Encode second field, validate that its memory contents match the encoded result
    Span<uint8_t> second_field_encoded_span {byte_span.subspan(math::bytes_to_contain(bits_encoded), kSecondFieldSizeBytes)};
    Span<uint8_t> second_value_span {reinterpret_cast<uint8_t*>(&second_value), kSecondFieldSizeBytes};

    result = encode(second, byte_span, bits_encoded);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(byte_spans_match(second_field_encoded_span, second_value_span));

    // Encode third field, validate that its memory contents match the encoded result
    Span<uint8_t> third_field_encoded_span {byte_span.subspan(math::bytes_to_contain(bits_encoded), kThirdFieldSizeBytes)};
    Span<uint8_t> third_value_span {reinterpret_cast<uint8_t*>(&third_value), kThirdFieldSizeBytes};

    result = encode(third, byte_span, bits_encoded);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(byte_spans_match(third_field_encoded_span, third_value_span));
}

/**
 * Test that values stored by different Fields are sequentially decoded from a byte buffer
 *
 */
TEST(Field, decode_sequential)
{
    using FirstField  = Field<int>;
    using SecondField = Field<uint16_t>;
    using ThirdField  = Field<float>;

    constexpr size_t kFirstFieldSizeBytes {math::bytes_to_contain(FirstField::kSizeBits)};
    constexpr size_t kSecondFieldSizeBytes {math::bytes_to_contain(SecondField::kSizeBits)};
    constexpr size_t kThirdFieldSizeBytes {math::bytes_to_contain(ThirdField::kSizeBits)};

    typename FirstField::value_type first_value {255};
    FirstField                      first {first_value};

    typename SecondField::value_type second_value {0xA5A5};
    SecondField                      second {second_value};

    typename ThirdField::value_type third_value {3.14};
    ThirdField                      third {third_value};

    constexpr size_t    kFieldsSizeBytes {kFirstFieldSizeBytes + kSecondFieldSizeBytes + kThirdFieldSizeBytes};
    uint8_t             bytes[kFieldsSizeBytes];
    Span<uint8_t const> byte_span {bytes, kFieldsSizeBytes};

    // Copy values in to byte buffer
    std::memcpy(bytes, reinterpret_cast<uint8_t const*>(&first), kFirstFieldSizeBytes);
    std::memcpy((bytes + kFirstFieldSizeBytes), reinterpret_cast<uint8_t const*>(&second), kSecondFieldSizeBytes);
    std::memcpy((bytes + kFirstFieldSizeBytes + kSecondFieldSizeBytes), reinterpret_cast<uint8_t const*>(&third),
                kThirdFieldSizeBytes);

    // Decode first field, validate that its stored value matches what is expected
    size_t     bits_decoded {0U};
    FirstField first_decoded;
    auto       result {decode(byte_span, bits_decoded, first_decoded)};
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(first_value, first_decoded.value);

    // Decode second field, perform same validation
    SecondField second_decoded;
    result = decode(byte_span, bits_decoded, second_decoded);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(second_value, second_decoded.value);

    // Ditto with the third
    ThirdField third_decoded;
    result = decode(byte_span, bits_decoded, third_decoded);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(third_value, third_decoded.value);

    ASSERT_EQ(math::bits_to_contain(kFieldsSizeBytes), bits_decoded);
}

/**
 * Test that the value stored by a Field is properly encoded in to a byte buffer when the start bit is at an offset
 *
 */
TEST(Field, encode_at_offset)
{
    using FieldType = Field<int>;

    constexpr static size_t kStartOffsetBits {3U};
    constexpr size_t        kFieldSizeBytes {math::bytes_to_contain(FieldType::kSizeBits)};
    constexpr size_t        kBufferSizeBytes {kFieldSizeBytes + 1U};
    constexpr static size_t kFinalOffsetBits {math::bits_to_contain(kBufferSizeBytes)};

    typename FieldType::value_type value {-255};
    FieldType                      field {value};

    uint8_t       bytes[kBufferSizeBytes];
    Span<uint8_t> byte_span {bytes, kBufferSizeBytes};
    size_t        bits_encoded {kStartOffsetBits};

    // Clear destination bytes before encoding
    static_cast<void>(std::memset(bytes, 0U, kBufferSizeBytes));

    // Encode field, validate that its memory contents match the encoded result, and check that the encoding begins at 1
    // byte from the beginning of the byte buffer
    Span<uint8_t> field_encoded_span {byte_span.subspan(1U, kFieldSizeBytes)};
    Span<uint8_t> value_span {reinterpret_cast<uint8_t*>(&value), kFieldSizeBytes};

    auto result {encode(field, byte_span, bits_encoded)};
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(bits_encoded, kFinalOffsetBits);
    EXPECT_TRUE(byte_spans_match(field_encoded_span, value_span));
}

/**
 * Test that the value stored by a Field is properly decoded from a byte buffer when the start bit is at an offset
 *
 */
TEST(Field, decode_at_offset)
{
    using FieldType = Field<uint8_t>;

    constexpr static size_t kStartOffsetBits {3U};
    constexpr size_t        kFieldSizeBytes {math::bytes_to_contain(FieldType::kSizeBits)};
    constexpr size_t        kBufferSizeBytes {kFieldSizeBytes + 1U};

    typename FieldType::value_type value {255};
    FieldType                      field {value};

    // Encoded value is located at next byte boundary after the start position
    uint8_t             bytes[kBufferSizeBytes] {0x00, 0xFF};
    Span<uint8_t const> byte_span {bytes, kBufferSizeBytes};

    // Decode field, validate that its stored value matches what is expected
    size_t    bits_decoded {kStartOffsetBits};
    FieldType field_decoded;
    auto      result {decode(byte_span, bits_decoded, field_decoded)};
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(value, field_decoded.value);

    constexpr static size_t kFinalOffsetBits {math::bits_to_contain(kBufferSizeBytes)};
    ASSERT_EQ(kFinalOffsetBits, bits_decoded);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  BitField tests              ////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Test that values stored by different unit BitFields are sequentially encoded in to a byte buffer
 *
 */
TEST(BitField, encode_bits)
{
    /*
        Encode the following fields in the order displayed below

                                        bytes
                                        0 |
          1   0   1   0   1   0   1   0   |
         ---------------------------------|
        | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8   |
                   1 bit, each

        The resultant byte span should equal: {0x55}
    */

    uint8_t       byte {0U};
    Span<uint8_t> byte_span {&byte, 1U};
    size_t        bits_encoded {0U};

    // Encode bit pattern a single bit at a time
    bool value {true};
    for (size_t i = 0; i < math::bits_to_contain(1U); i++)
    {
        Bit  bit {value};
        auto result {encode(bit, byte_span, bits_encoded)};
        EXPECT_TRUE(result.IsSuccess());
        EXPECT_EQ(bits_encoded, i + 1);

        // Invert every bit from the former
        value = !value;
    }

    ASSERT_EQ(0x55, byte);
}

/**
 * Test that values stored by different unit BitFields are sequentially decoded from a byte buffer
 *
 */
TEST(BitField, decode_bits)
{
    /*
        The following fields are encoded in to a byte buffer as below:

                                        bytes
                                        0 |
          1   0   1   0   1   0   1   0   |
         ---------------------------------|
        | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8   |
                   1 bit, each

        The resultant byte span should equal: {0x55}
    */

    uint8_t             byte {0x55};
    Span<uint8_t const> byte_span {&byte, 1U};

    // Decode bit pattern a single bit at a time
    size_t bits_decoded {0U};
    for (size_t i = 0; i < math::bits_to_contain(1U); i++)
    {
        Bit  bit;
        auto result {decode(byte_span, bits_decoded, bit)};
        EXPECT_TRUE(result.IsSuccess());
        EXPECT_EQ(bits_decoded, i + 1);

        bool expected_value {static_cast<bool>((byte >> i) & 0x01)};
        ASSERT_EQ(expected_value, bit.value) << "Bit position " << i;
    }
}

/**
 * Test that values stored by different BitFields, each smaller than a byte, are sequentially encoded in to a byte buffer
 *
 */
TEST(BitField, encode_sub_byte_values)
{
    /*
        Encode the following fields in the order displayed below

                                        bytes
                                        0 | 1
          0   1   1   0   1   0   1   1   |   0   1   0   1   0   1   1   0
         ---------------------------------|---------------------------------
        |     1     |         2      |    ^          2          |     4     |
            3 bits           4 bits                 6 bits           3 bits
            (0x06)           (0x0A)                 (0x15)           (0x03)

        The resultant byte span should equal: {0xD6, 0x6A}
    */

    using FirstBitField = BitField<3>;
    typename FirstBitField::value_type first_value {0x06};

    using SecondBitField = BitField<4>;
    typename SecondBitField::value_type second_value {0x0A};

    using ThirdBitField = BitField<6>;
    typename ThirdBitField::value_type third_value {0x15};

    using FourthBitField = BitField<3>;
    typename FourthBitField::value_type fourth_value {0x03};

    constexpr size_t kEncodingSizeBytes {math::bytes_to_contain(FirstBitField::kSizeBits + SecondBitField::kSizeBits +
                                                                ThirdBitField::kSizeBits + FourthBitField::kSizeBits)};
    uint8_t          bytes[kEncodingSizeBytes];
    Span<uint8_t>    byte_span {bytes, kEncodingSizeBytes};
    size_t           bits_encoded {0U};

    // Clear destination bytes before encoding
    static_cast<void>(std::memset(bytes, 0U, kEncodingSizeBytes));

    FirstBitField first {first_value};
    auto          result {encode(first, byte_span, bits_encoded)};
    ASSERT_TRUE(result.IsSuccess());

    SecondBitField second {second_value};
    size_t         pre_second_bits_encoded {bits_encoded};
    result = encode(second, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    ThirdBitField third {third_value};
    size_t        pre_third_bits_encoded {bits_encoded};
    result = encode(third, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    FourthBitField fourth {fourth_value};
    size_t         pre_fourth_bits_encoded {bits_encoded};
    result = encode(fourth, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    uint8_t       check_bytes[kEncodingSizeBytes] {0xD6, 0x6A};
    Span<uint8_t> check_byte_span {check_bytes, kEncodingSizeBytes};
    EXPECT_TRUE(byte_spans_match(check_byte_span, byte_span));
}

/**
 * Test that values stored by different BitFields, each smaller than a byte, are sequentially decoded from a byte buffer
 *
 */
TEST(BitField, decode_sub_byte_values)
{
    /*
        The following fields are encoded in to a byte buffer as below:

                                        bytes
                                        0 | 1
          0   1   1   0   1   0   1   1   |   0   1   0   1   0   1   1   0
         ---------------------------------|---------------------------------
        |     1     |         2      |    ^          2          |     4     |
            3 bits           4 bits                 6 bits           3 bits
            (0x06)           (0x0A)                 (0x15)           (0x03)

        The resultant byte span should equal: {0xD6, 0x6A}
    */

    using FirstBitField = BitField<3>;
    typename FirstBitField::value_type first_value {0x06};

    using SecondBitField = BitField<4>;
    typename SecondBitField::value_type second_value {0x0A};

    using ThirdBitField = BitField<6>;
    typename ThirdBitField::value_type third_value {0x15};

    using FourthBitField = BitField<3>;
    typename FourthBitField::value_type fourth_value {0x03};

    constexpr size_t kEncodingSizeBytes {math::bytes_to_contain(FirstBitField::kSizeBits + SecondBitField::kSizeBits +
                                                                ThirdBitField::kSizeBits + FourthBitField::kSizeBits)};

    uint8_t             bytes[kEncodingSizeBytes] {0xD6, 0x6A};
    Span<uint8_t const> byte_span {bytes, kEncodingSizeBytes};

    // Perform decodings
    size_t bits_decoded {0U};

    FirstBitField first_decoded;
    auto          result {decode(byte_span, bits_decoded, first_decoded)};
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(first_value, first_decoded.value);

    SecondBitField second_decoded;
    result = decode(byte_span, bits_decoded, second_decoded);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(second_value, second_decoded.value);

    ThirdBitField third_decoded;
    result = decode(byte_span, bits_decoded, third_decoded);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(third_value, third_decoded.value);

    FourthBitField fourth_decoded;
    result = decode(byte_span, bits_decoded, fourth_decoded);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(fourth_value, fourth_decoded.value);

    ASSERT_EQ(math::bits_to_contain(kEncodingSizeBytes), bits_decoded);
}

/**
 * Test that values stored by different BitFields, each larger than a byte, are sequentially encoded in to a byte buffer
 *
 */
TEST(BitField, encode_super_byte_values)
{
    /*
        Encode the following fields in the order displayed below

                                            bytes
                                        0 | 1
          0   1   1   0   1   0   1   1   |   0   1   0   1   0   1   1   0
         ---------------------------------|---------------------------------
        |               1                 ^                     |     2
                      13 bits                                       17 bits
                      (0x0AD6)                                      (0x56B3)

                                        bytes
                                        2 | 3
          0   1   1   0   1   0   1   1   |   0   1   0   1   0   0   1   1
         ---------------------------------|---------------------------------
                      2 cont.             ^                         |   3
                                                                       9 bits
                                                                      (0x015B)

                                        bytes
                                        4 | 5
          0   1   1   0   1   0   1   1   |   0   1   0   1   0   0   1   1
         ---------------------------------|---------------------------------
                      3 cont.      |      ^               4                 |
                                                         9 bits
                                                         (0x0195)

        The resultant byte span should equal: {0xD6, 0x6A, 0xD6, 0xCA, 0xD6, 0xCA}
    */

    using FirstBitField = BitField<13>;
    typename FirstBitField::value_type first_value {0x0AD6};

    using SecondBitField = BitField<17>;
    typename SecondBitField::value_type second_value {0x56B3};

    using ThirdBitField = BitField<9>;
    typename ThirdBitField::value_type third_value {0x015B};

    using FourthBitField = BitField<9>;
    typename FourthBitField::value_type fourth_value {0x0195};

    constexpr size_t kEncodingSizeBytes {math::bytes_to_contain(FirstBitField::kSizeBits + SecondBitField::kSizeBits +
                                                                ThirdBitField::kSizeBits + FourthBitField::kSizeBits)};
    uint8_t          bytes[kEncodingSizeBytes];
    Span<uint8_t>    byte_span {bytes, kEncodingSizeBytes};
    size_t           bits_encoded {0U};

    // Clear destination bytes before encoding
    static_cast<void>(std::memset(bytes, 0U, kEncodingSizeBytes));

    FirstBitField first {first_value};
    auto          result {encode(first, byte_span, bits_encoded)};
    ASSERT_TRUE(result.IsSuccess());

    SecondBitField second {second_value};
    size_t         pre_second_bits_encoded {bits_encoded};
    result = encode(second, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    ThirdBitField third {third_value};
    size_t        pre_third_bits_encoded {bits_encoded};
    result = encode(third, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    FourthBitField fourth {fourth_value};
    size_t         pre_fourth_bits_encoded {bits_encoded};
    result = encode(fourth, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    ASSERT_EQ(math::bits_to_contain(kEncodingSizeBytes), bits_encoded);

    uint8_t       check_bytes[kEncodingSizeBytes] {0xD6, 0x6A, 0xD6, 0xCA, 0xD6, 0xCA};
    Span<uint8_t> check_byte_span {check_bytes, kEncodingSizeBytes};
    ASSERT_TRUE(byte_spans_match(check_byte_span, byte_span));
}

/**
 * Test that values stored by different BitFields, each larger than a byte, are sequentially decoded from a byte buffer
 *
 */
TEST(BitField, decode_super_byte_values)
{
    /*
        The following fields are encoded in to a byte buffer as below:

                                            bytes
                                        0 | 1
          0   1   1   0   1   0   1   1   |   0   1   0   1   0   1   1   0
         ---------------------------------|---------------------------------
        |               1                 ^                     |     2
                      13 bits                                       17 bits
                      (0x0AD6)                                      (0x56B3)

                                        bytes
                                        2 | 3
          0   1   1   0   1   0   1   1   |   0   1   0   1   0   0   1   1
         ---------------------------------|---------------------------------
                      2 cont.             ^                         |   3
                                                                       9 bits
                                                                      (0x015B)

                                        bytes
                                        4 | 5
          0   1   1   0   1   0   1   1   |   0   1   0   1   0   0   1   1
         ---------------------------------|---------------------------------
                      3 cont.      |      ^               4                 |
                                                         9 bits
                                                         (0x0195)

        The resultant byte span should equal: {0xD6, 0x6A, 0xD6, 0xCA, 0xD6, 0xCA}
    */

    using FirstBitField = BitField<13>;
    typename FirstBitField::value_type first_value {0x0AD6};

    using SecondBitField = BitField<17>;
    typename SecondBitField::value_type second_value {0x56B3};

    using ThirdBitField = BitField<9>;
    typename ThirdBitField::value_type third_value {0x015B};

    using FourthBitField = BitField<9>;
    typename FourthBitField::value_type fourth_value {0x0195};

    constexpr size_t kEncodingSizeBytes {math::bytes_to_contain(FirstBitField::kSizeBits + SecondBitField::kSizeBits +
                                                                ThirdBitField::kSizeBits + FourthBitField::kSizeBits)};

    uint8_t             bytes[kEncodingSizeBytes] {0xD6, 0x6A, 0xD6, 0xCA, 0xD6, 0xCA};
    Span<uint8_t const> byte_span {bytes, kEncodingSizeBytes};

    // Perform decodings
    size_t bits_decoded {0U};

    FirstBitField first_decoded;
    auto          result {decode(byte_span, bits_decoded, first_decoded)};
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(first_value, first_decoded.value);

    SecondBitField second_decoded;
    result = decode(byte_span, bits_decoded, second_decoded);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(second_value, second_decoded.value);

    ThirdBitField third_decoded;
    result = decode(byte_span, bits_decoded, third_decoded);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(third_value, third_decoded.value);

    FourthBitField fourth_decoded;
    result = decode(byte_span, bits_decoded, fourth_decoded);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_EQ(fourth_value, fourth_decoded.value);

    ASSERT_EQ(math::bits_to_contain(kEncodingSizeBytes), bits_decoded);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  ConstBitField tests             ////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Test that values stored by different unit ConstBitFields are sequentially encoded in to a byte buffer
 *
 */
TEST(ConstBitField, encode_bits)
{
    /*
        Encode the following fields in the order displayed below

                                        bytes
                                        0 |
          1   0   1   0   1   0   1   0   |
         ---------------------------------|
        | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8   |
                   1 bit, each

        The resultant byte span should equal: {0x55}
    */

    uint8_t       byte {0U};
    Span<uint8_t> byte_span {&byte, 1U};
    size_t        bits_encoded {0U};

    // Encode bit pattern, a single bit at a time
    bool value {true};
    for (size_t i = 0; i < math::bits_to_contain(1U); i++)
    {
        ConstBit bit {value};
        auto     result {encode(bit, byte_span, bits_encoded)};
        ASSERT_TRUE(result.IsSuccess());
        ASSERT_EQ(bits_encoded, i + 1);

        // Invert every bit from the former
        value = !value;
    }

    ASSERT_EQ(0x55, byte);
}

/**
 * Test that values stored by different ConstBitFields, each smaller than a byte, are sequentially encoded in to a byte buffer
 *
 */
TEST(ConstBitField, encode_sub_byte_values)
{
    /*
        Encode the following fields in the order displayed below

                                        bytes
                                        0 | 1
          0   1   1   0   1   0   1   1   |   0   1   0   1   0   1   1   0
         ---------------------------------|---------------------------------
        |     1     |         2      |    ^          2          |     4     |
            3 bits           4 bits                 6 bits           3 bits
            (0x06)           (0x0A)                 (0x15)           (0x03)

        The resultant byte span should equal: {0xD6, 0x6A}
    */

    using FirstBitField = ConstBitField<3>;
    typename FirstBitField::value_type first_value {0x06};

    using SecondBitField = ConstBitField<4>;
    typename SecondBitField::value_type second_value {0x0A};

    using ThirdBitField = ConstBitField<6>;
    typename ThirdBitField::value_type third_value {0x15};

    using FourthBitField = ConstBitField<3>;
    typename FourthBitField::value_type fourth_value {0x03};

    constexpr size_t kEncodingSizeBytes {math::bytes_to_contain(FirstBitField::kSizeBits + SecondBitField::kSizeBits +
                                                                ThirdBitField::kSizeBits + FourthBitField::kSizeBits)};
    uint8_t          bytes[kEncodingSizeBytes];
    Span<uint8_t>    byte_span {bytes, kEncodingSizeBytes};
    size_t           bits_encoded {0U};

    // Clear destination bytes before encoding
    static_cast<void>(std::memset(bytes, 0U, kEncodingSizeBytes));

    FirstBitField first {first_value};
    auto          result {encode(first, byte_span, bits_encoded)};
    ASSERT_TRUE(result.IsSuccess());

    SecondBitField second {second_value};
    size_t         pre_second_bits_encoded {bits_encoded};
    result = encode(second, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    ThirdBitField third {third_value};
    size_t        pre_third_bits_encoded {bits_encoded};
    result = encode(third, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    FourthBitField fourth {fourth_value};
    size_t         pre_fourth_bits_encoded {bits_encoded};
    result = encode(fourth, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    uint8_t       check_bytes[kEncodingSizeBytes] {0xD6, 0x6A};
    Span<uint8_t> check_byte_span {check_bytes, kEncodingSizeBytes};
    EXPECT_TRUE(byte_spans_match(check_byte_span, byte_span));
}

/**
 * Test that values stored by different ConstBitFields, each larger than a byte, are sequentially encoded in to a byte buffer
 *
 */
TEST(ConstBitField, encode_super_byte_values)
{
    /*
        Encode the following fields in the order displayed below

                                            bytes
                                        0 | 1
        0   1   1   0   1   0   1   1   |   0   1   0   1   0   1   1   0
        ---------------------------------|---------------------------------
        |               1                 ^                     |     2
                    13 bits                                       17 bits
                    (0x0AD6)                                      (0x56B3)

                                        bytes
                                        2 | 3
        0   1   1   0   1   0   1   1   |   0   1   0   1   0   0   1   1
        ---------------------------------|---------------------------------
                    2 cont.             ^                         |   3
                                                                    9 bits
                                                                    (0x015B)

                                        bytes
                                        4 | 5
        0   1   1   0   1   0   1   1   |   0   1   0   1   0   0   1   1
        ---------------------------------|---------------------------------
                    3 cont.      |      ^               4                 |
                                                        9 bits
                                                        (0x0195)

        The resultant byte span should equal: {0xD6, 0x6A, 0xD6, 0xCA, 0xD6, 0xCA}
    */

    using FirstBitField = ConstBitField<13>;
    typename FirstBitField::value_type first_value {0x0AD6};

    using SecondBitField = ConstBitField<17>;
    typename SecondBitField::value_type second_value {0x56B3};

    using ThirdBitField = ConstBitField<9>;
    typename ThirdBitField::value_type third_value {0x015B};

    using FourthBitField = ConstBitField<9>;
    typename FourthBitField::value_type fourth_value {0x0195};

    constexpr size_t kEncodingSizeBytes {math::bytes_to_contain(FirstBitField::kSizeBits + SecondBitField::kSizeBits +
                                                                ThirdBitField::kSizeBits + FourthBitField::kSizeBits)};
    uint8_t          bytes[kEncodingSizeBytes];
    Span<uint8_t>    byte_span {bytes, kEncodingSizeBytes};
    size_t           bits_encoded {0U};

    // Clear destination bytes before encoding
    static_cast<void>(std::memset(bytes, 0U, kEncodingSizeBytes));

    FirstBitField first {first_value};
    auto          result {encode(first, byte_span, bits_encoded)};
    ASSERT_TRUE(result.IsSuccess());

    SecondBitField second {second_value};
    size_t         pre_second_bits_encoded {bits_encoded};
    result = encode(second, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    ThirdBitField third {third_value};
    size_t        pre_third_bits_encoded {bits_encoded};
    result = encode(third, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    FourthBitField fourth {fourth_value};
    size_t         pre_fourth_bits_encoded {bits_encoded};
    result = encode(fourth, byte_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    uint8_t       check_bytes[kEncodingSizeBytes] {0xD6, 0x6A, 0xD6, 0xCA, 0xD6, 0xCA};
    Span<uint8_t> check_byte_span {check_bytes, kEncodingSizeBytes};
    EXPECT_TRUE(byte_spans_match(check_byte_span, byte_span));
}
