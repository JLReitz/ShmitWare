#include "Help.hpp"

#include <Core/Data/Encode.hpp>
#include <Core/Math/Memory.hpp>
#include <Core/Span.hpp>

#include <gtest/gtest.h>

#include <cstring>
#include <type_traits>

using namespace shmit;
using namespace shmit::data;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Encode tests                ////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Test that single values are properly encoded in to a byte buffer
 *
 */
TEST(Encode, single_values)
{
    using First  = int8_t;
    using Second = uint32_t;
    using Third  = float;

    First  first {-69};
    Second second {0x0ABCDEF0};
    Third  third {3.14};

    Span<uint8_t> first_value_span {reinterpret_cast<uint8_t*>(&first), footprint_size_bytes_v<First>};
    Span<uint8_t> second_value_span {reinterpret_cast<uint8_t*>(&second), footprint_size_bytes_v<Second>};
    Span<uint8_t> third_value_span {reinterpret_cast<uint8_t*>(&third), footprint_size_bytes_v<Third>};

    // Perform and check first encoding
    uint8_t       first_encoded_bytes[footprint_size_bytes_v<First>];
    Span<uint8_t> first_encoded_span {first_encoded_bytes, footprint_size_bytes_v<First>};
    size_t        first_encoded_bit_count {0U};
    auto          result {encode(first, first_encoded_span, first_encoded_bit_count)};
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(footprint_size_bits_v<First>, first_encoded_bit_count);
    EXPECT_TRUE(byte_spans_match(first_value_span, first_encoded_span));

    // Perform and check second encoding
    uint8_t       second_encoded_bytes[footprint_size_bytes_v<Second>];
    Span<uint8_t> second_encoded_span {second_encoded_bytes, footprint_size_bytes_v<Second>};
    size_t        second_encoded_bit_count {0U};
    result = encode(second, second_encoded_span, second_encoded_bit_count);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(footprint_size_bits_v<Second>, second_encoded_bit_count);
    EXPECT_TRUE(byte_spans_match(second_value_span, second_encoded_span));

    // Perform and check third encoding
    uint8_t       third_encoded_bytes[footprint_size_bytes_v<Third>];
    Span<uint8_t> third_encoded_span {second_encoded_bytes, footprint_size_bytes_v<Third>};
    size_t        third_encoded_bit_count {0U};
    result = encode(third, third_encoded_span, third_encoded_bit_count);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(footprint_size_bits_v<Third>, third_encoded_bit_count);
    EXPECT_TRUE(byte_spans_match(third_value_span, third_encoded_span));
}

/**
 * Test that values are sequentially encoded in to a byte buffer
 *
 */
TEST(Encode, sequential_values)
{
    using First  = uint8_t;
    using Second = uint32_t;

    First  first {0xFF};
    Second second {0x0ABCDEF0};

    constexpr size_t kByteBufferSize {footprint_size_bytes_v<First> + footprint_size_bytes_v<Second>};
    uint8_t          bytes[kByteBufferSize];
    Span<uint8_t>    encoded_span {bytes, kByteBufferSize};
    size_t           bits_encoded {0U};

    // Perform encodings
    auto result {encode(first, encoded_span, bits_encoded)};
    ASSERT_TRUE(result.IsSuccess());
    result = encode(second, encoded_span, bits_encoded);
    ASSERT_TRUE(result.IsSuccess());

    // Check encoding
    ASSERT_EQ(math::bits_to_contain(kByteBufferSize), bits_encoded);

    uint8_t       expected_bytes[5U] {0xFF, 0xF0, 0xDE, 0xBC, 0x0A};
    Span<uint8_t> expected_span {expected_bytes, 5U};
    EXPECT_TRUE(byte_spans_match(expected_span, encoded_span));
}

/**
 * Test that values are properly encoded in to a byte buffer when the start bit is at a nonzero offset
 *
 */
TEST(Encode, at_offset)
{
    constexpr size_t kStartEncodingOffsetBits {3U};

    uint8_t value {0xFF};

    uint8_t       bytes[2U];
    Span<uint8_t> encoded_span {bytes, 2U};
    size_t        bits_encoded {kStartEncodingOffsetBits};
    std::memset(bytes, 0U, 2U);

    auto result {encode(value, encoded_span, bits_encoded)};
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(16, bits_encoded);

    // Expect encoded value to be displaced to next byte boundary
    uint8_t       expected_bytes[2U] {0x00, 0xFF};
    Span<uint8_t> expected_span {expected_bytes, 2U};
    EXPECT_TRUE(byte_spans_match(expected_span, encoded_span));
}

/**
 * Test that values are not encoded when doing so would overflow the destination byte buffer
 *
 */
TEST(Encode, avoids_overflow)
{
    using First  = uint8_t;
    using Second = uint32_t;

    First  first {0xFF};
    Second second {0xFFFFFFFF};

    // Create a destination buffer that is too small to contain the second type
    constexpr size_t kByteBufferSize {2U};
    uint8_t          bytes[kByteBufferSize];
    Span<uint8_t>    encoded_span {bytes, kByteBufferSize};
    size_t           bits_encoded {0U};
    std::memset(bytes, 0U, 2U);

    // Perform first encoding, expect it to succeed
    auto result {encode(first, encoded_span, bits_encoded)};
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(footprint_size_bits_v<First>, bits_encoded);

    // Perform second encoding, expect it to fail
    result = encode(second, encoded_span, bits_encoded);
    ASSERT_TRUE(result.IsFailure());

    // Ensure that bits_encoded still only accounts for the first value
    EXPECT_EQ(footprint_size_bits_v<First>, bits_encoded);

    // Ensure that only the first value was encoded
    uint8_t       expected_bytes[2U] {0xFF, 0x00};
    Span<uint8_t> expected_span {expected_bytes, 2U};
    EXPECT_TRUE(byte_spans_match(expected_span, encoded_span));
}