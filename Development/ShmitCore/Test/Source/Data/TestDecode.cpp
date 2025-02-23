#include "Help.hpp"

#include <ShmitCore/Data/Decode.hpp>
#include <ShmitCore/Math/Memory.hpp>
#include <ShmitCore/Span.hpp>

#include <gtest/gtest.h>

#include <cstring>
#include <type_traits>

using namespace shmit;
using namespace shmit::data;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Decode tests                ////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Test that single values are properly decoded from a byte buffer
 *
 */
TEST(Decode, single_values)
{
    using First  = int8_t;
    using Second = uint32_t;
    using Third  = float;

    First  first {-69};
    Second second {0x0ABCDEF0};
    Third  third {3.14};

    Span<uint8_t const> first_value_span {reinterpret_cast<uint8_t const*>(&first), footprint_size_bytes_v<First>};
    Span<uint8_t const> second_value_span {reinterpret_cast<uint8_t const*>(&second), footprint_size_bytes_v<Second>};
    Span<uint8_t const> third_value_span {reinterpret_cast<uint8_t const*>(&third), footprint_size_bytes_v<Third>};

    // Perform and check first decoding
    First  first_decoded;
    size_t first_decoded_bit_count {0U};
    auto   result {decode(first_value_span, first_decoded_bit_count, first_decoded)};
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(footprint_size_bits_v<First>, first_decoded_bit_count);
    EXPECT_EQ(first, first_decoded);

    // Perform and check second decoding
    Second second_decoded;
    size_t second_decoded_bit_count {0U};
    result = decode(second_value_span, second_decoded_bit_count, second_decoded);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(footprint_size_bits_v<Second>, second_decoded_bit_count);
    EXPECT_EQ(second, second_decoded);

    // Perform and check third decoding
    Third  third_decoded;
    size_t third_decoded_bit_count {0U};
    result = decode(third_value_span, third_decoded_bit_count, third_decoded);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(footprint_size_bits_v<Third>, third_decoded_bit_count);
    EXPECT_EQ(third, third_decoded);
}

/**
 * Test that values are sequentially decoded from a byte buffer
 *
 */
TEST(Decode, sequential_values)
{
    using First  = uint8_t;
    using Second = uint32_t;

    First  first {0xFF};
    Second second {0x0ABCDEF0};

    constexpr size_t    kByteBufferSize {footprint_size_bytes_v<First> + footprint_size_bytes_v<Second>};
    uint8_t             bytes[kByteBufferSize];
    Span<uint8_t const> encoded_span {bytes, kByteBufferSize};

    // Copy values in to encoded buffer
    std::memcpy(bytes, reinterpret_cast<uint8_t const*>(&first), footprint_size_bytes_v<First>);
    std::memcpy((bytes + footprint_size_bytes_v<First>), reinterpret_cast<uint8_t const*>(&second),
                footprint_size_bytes_v<Second>);

    // Perform decodings
    First  first_decoded;
    Second second_decoded;
    size_t bits_decoded {0U};
    auto   result {decode(encoded_span, bits_decoded, first_decoded)};
    ASSERT_TRUE(result.IsSuccess());
    result = decode(encoded_span, bits_decoded, second_decoded);
    ASSERT_TRUE(result.IsSuccess());

    // Check decoding
    EXPECT_EQ(math::bits_to_contain(kByteBufferSize), bits_decoded);

    EXPECT_EQ(first, first_decoded);
    EXPECT_EQ(second, second_decoded);
}

/**
 * Test that values are properly decoded from a byte buffer when the start bit is at a nonzero offset
 *
 */
TEST(Decode, at_offset)
{
    constexpr size_t kStartDecodingOffsetBits {3U};

    uint8_t value {0xFF};

    // Encoded value is located at next byte boundary after the start position
    uint8_t             bytes[2U] {0x00, value};
    Span<uint8_t const> encoded_span {bytes, 2U};

    // Perform decoding
    uint8_t decoded_value {0U};
    size_t  bits_decoded {kStartDecodingOffsetBits};
    auto    result {decode(encoded_span, bits_decoded, decoded_value)};

    // Check decoding
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(16U, bits_decoded);
    ASSERT_EQ(value, decoded_value);
}

/**
 * Test that values are not decoded when doing so would underflow the source byte buffer
 *
 */
TEST(Decode, avoids_underflow)
{
    using First  = uint8_t;
    using Second = uint32_t;

    First first {0xFF};

    // Create a source buffer that is too small to contain the second type
    constexpr size_t    kByteBufferSize {2U};
    uint8_t             bytes[kByteBufferSize] {first, 0x00};
    Span<uint8_t const> encoded_span {bytes, kByteBufferSize};

    // Perform first decoding, expect it to succeed
    First  first_decoded;
    size_t bits_decoded {0U};
    auto   result {decode(encoded_span, bits_decoded, first_decoded)};
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(footprint_size_bits_v<First>, bits_decoded);

    // Perform second decoding, expect it to fail
    Second second_decoded;
    result = decode(encoded_span, bits_decoded, second_decoded);
    ASSERT_TRUE(result.IsFailure());

    // Ensure that bits_decoded still only accounts for the first value
    EXPECT_EQ(footprint_size_bits_v<First>, bits_decoded);

    // Check the first decoded value
    ASSERT_EQ(first, first_decoded);
}
