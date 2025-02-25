#include "Help.hpp"

#include <Core/Data/Packet.hpp>
#include <Core/Span.hpp>

#include <gtest/gtest.h>

#include <cstring>
#include <type_traits>

using namespace shmit;
using namespace shmit::data;

// Test against a Packet smaller than a byte
// They could exist... we don't judge the little ones
using FunSizePacket = packet_t<Bit, Bit, Bit, Bit, Bit>;
FunSizePacket::value_type_t<0U> fun_size_packet_field_1 {true};
FunSizePacket::value_type_t<1U> fun_size_packet_field_2 {false};
FunSizePacket::value_type_t<2U> fun_size_packet_field_3 {true};
FunSizePacket::value_type_t<3U> fun_size_packet_field_4 {false};
FunSizePacket::value_type_t<4U> fun_size_packet_field_5 {true};
/*
    The Packet structure declared above would be represented as follows:

                                    bytes
                                    0 |
      1   0   1   0   1   0   0   0   |
     ---------------------------------|
    | 1 | 2 | 3 | 4 | 5 |   padding
          1 bit, each       3 bits

    The resultant byte should equal: {0x15}
*/
constexpr size_t kFunSizePacketExpectedSizeBits {8U};
constexpr size_t kFunSizePacketExpectedSizeBytes {1u};
uint8_t          fun_size_packet_expected_bytes[kFunSizePacketExpectedSizeBytes] {0x15};

FunSizePacket fun_size_packet {fun_size_packet_field_1, fun_size_packet_field_2, fun_size_packet_field_3,
                               fun_size_packet_field_4, fun_size_packet_field_5};

// Test against a Packet with some gaps between the values
using LooselyPackedPacket = packet_t<Bit, uint8_t, bool, BitField<14>, uint16_t>;
LooselyPackedPacket::value_type_t<0U> loosely_packed_packet_field_1 {false};
LooselyPackedPacket::value_type_t<1U> loosely_packed_packet_field_2 {255};
LooselyPackedPacket::value_type_t<2U> loosely_packed_packet_field_3 {true};
LooselyPackedPacket::value_type_t<3U> loosely_packed_packet_field_4 {0x1FFF};
LooselyPackedPacket::value_type_t<4U> loosely_packed_packet_field_5 {0xA55A};
/*
    The Packet structure declared above would be represented as follows:

                                    bytes
                                    0 | 1
      0   0   0   0   0   0   0   0   |   1   1   1   1   1   1   1   1
     ---------------------------------|---------------------------------
    | 1 |          padding            |                  2              |
    1 bit          7 bits                              8 bits
    (0x0)                                              (0xFF)

                                    bytes
                                    2 | 3
      1   0   0   0   0   0   0   0   |   1   1   1   1   1   1   1   1
     ---------------------------------|---------------------------------
                    3                 |                 4
                  8 bits                              14 bits
                  (0x01)                              (0x1FFF)

                                    bytes
                                    4 | 5
      1   1   1   1   1   0   0   0   |   0   1   0   1   1   0   1   0
     ---------------------------------|---------------------------------
                4 cont.     |         |             5
                                ^ padding         16 bits
                                  2 bits          (0xA55A)

                                    bytes
                                    6 |
      1   0   1   0   0   1   0   1   |
     ---------------------------------|
                  5 cont.             |

    The resultant byte span should equal: {0x00, 0xFF, 0x01, 0xFF, 0x1F, 0x5A, 0xA5}
*/
constexpr size_t kLooselyPackedPacketExpectedSizeBits {56U};
constexpr size_t kLooselyPackedPacketExpectedSizeBytes {7U};
uint8_t          loosely_packed_packet_expected_bytes[kLooselyPackedPacketExpectedSizeBytes] {0x00, 0xFF, 0x01, 0xFF,
                                                                                              0x1F, 0x5A, 0xA5};

LooselyPackedPacket loosely_packed_packet {loosely_packed_packet_field_1, loosely_packed_packet_field_2,
                                           loosely_packed_packet_field_3, loosely_packed_packet_field_4,
                                           loosely_packed_packet_field_5};

// Test against a Packet with no wiggle room
using TightlyPackedPacket = packet_t<uint8_t, BitField<7>, Bit, uint16_t, BitField<20>, BitField<36>>;
TightlyPackedPacket::value_type_t<0U> tightly_packed_packet_field_1 {0xA5};
TightlyPackedPacket::value_type_t<1U> tightly_packed_packet_field_2 {127};
TightlyPackedPacket::value_type_t<2U> tightly_packed_packet_field_3 {false};
TightlyPackedPacket::value_type_t<3U> tightly_packed_packet_field_4 {0x55AA};
TightlyPackedPacket::value_type_t<4U> tightly_packed_packet_field_5 {0xEDCBA};
TightlyPackedPacket::value_type_t<5U> tightly_packed_packet_field_6 {0x321ABCDEF};
/*
    The Packet structure declared above would be represented as follows:

                                    bytes
                                    0 | 1
      1   0   1   0   0   1   0   1   |   1   1   1   1   1   1   1   0
     ---------------------------------|---------------------------------
    |               1                 |             2               | 3 |
                   8 bits                          7 bits            1 bit
                   (0xA5)                          (0x7F)            (0x00)

                                    bytes
                                    2 | 3
      0   1   0   1   0   1   0   1   |   1   0   1   0   1   0   1   0
     ---------------------------------|---------------------------------
    |                     4           ^                                 |
                        16 bits
                        (0x55AA)

                                    bytes
                                    4 | 5
      0   1   0   1   1   1   0   1   |   0   0   1   1   1   0   1   1
     ---------------------------------|---------------------------------
    |                                 ^       5
                                            20 bits
                                           (0xEDCBA)

                                    bytes
                                    6 | 7
      0   1   1   1   1   1   1   1   |   0   1   1   1   1   0   1   1
     ---------------------------------|---------------------------------
                    |                 ^       6
                                            36 bits
                                         (0x321ABCDEF)

                                    bytes
                                    8 | 9
      0   0   1   1   1   1   0   1   |   0   1   0   1   1   0   0   0
     ---------------------------------|---------------------------------
                  6 cont.             ^



                                    bytes
                                   10 |
      0   1   0   0   1   1   0   0   |
     ---------------------------------|
                  6 cont.             |

    The resultant byte span should equal: {0xA5, 0x7F, 0xAA, 0x55, 0xBA, 0xDC, 0xFE, 0xDE, 0xBC, 0x1A, 0x32}
*/
constexpr size_t kTightlyPackedPacketExpectedSizeBits {88U};
constexpr size_t kTightlyPackedPacketExpectedSizeBytes {11U};
uint8_t tightly_packed_packet_expected_bytes[kTightlyPackedPacketExpectedSizeBytes] {0xA5, 0x7F, 0xAA, 0x55, 0xBA, 0xDC,
                                                                                     0xFE, 0xDE, 0xBC, 0x1A, 0x32};

TightlyPackedPacket tightly_packed_packet {tightly_packed_packet_field_1, tightly_packed_packet_field_2,
                                           tightly_packed_packet_field_3, tightly_packed_packet_field_4,
                                           tightly_packed_packet_field_5, tightly_packed_packet_field_6};

// Test against packet not quite ending on a byte boundary
using JunkInTheTrunkPacket = packet_t<BitField<29>, BitField<11>, uint32_t, Bit>;
JunkInTheTrunkPacket::value_type_t<0U> junk_in_the_trunk_packet_field_1 {0x1F7E0A5A};
JunkInTheTrunkPacket::value_type_t<1U> junk_in_the_trunk_packet_field_2 {1024U};
JunkInTheTrunkPacket::value_type_t<2U> junk_in_the_trunk_packet_field_3 {0x55AA55AA};
JunkInTheTrunkPacket::value_type_t<3U> junk_in_the_trunk_packet_field_4 {true};
/*
    The Packet structure declared above would be represented as follows:

                                    bytes
                                    0 | 1
      0   1   0   1   1   0   1   0   |   0   1   0   1   0   0   0   0
     ---------------------------------|---------------------------------
    |               1                 ^
                 29 bits
               (0x1F7E0A5A)

                                    bytes
                                    2 | 3
      0   1   1   1   1   1   1   0   |   1   1   1   1   1   0   0   0
     ---------------------------------|---------------------------------
                    1 cont.           ^                     |     2
                                                                11 bits
                                                                (0x400)

                                    bytes
                                    4 | 5
      0   0   0   0   0   0   0   1   |   0   1   0   1   0   1   0   1
     ---------------------------------|---------------------------------
                  2 cont.             |                 3
                                                     32 bits
                                                   (0x55AA55AA)

                                    bytes
                                    6 | 7
      1   0   1   0   1   0   1   0   |   0   1   0   1   0   1   0   1
     ---------------------------------|---------------------------------
                  3 cont.             ^



                                    bytes
                                    8 | 9
      1   0   1   0   1   0   1   0   |   1   0   0   0   0   0   0   0
     ---------------------------------|---------------------------------
                  3 cont.             |   4 |         padding
                                        1 bit         7 bits
                                        (0x01)

    The resultant byte span should equal: {0x5A, 0x0A, 0x7E, 0x1F, 0x80, 0xAA, 0x55, 0xAA, 0x55, 0x01}
*/
constexpr size_t kJunkInTheTrunkPacketExpectedSizeBits {80U};
constexpr size_t kJunkInTheTrunkPacketExpectedSizeBytes {10U};
uint8_t junk_in_the_trunk_packet_expected_bytes[kJunkInTheTrunkPacketExpectedSizeBytes] {0x5A, 0x0A, 0x7E, 0x1F, 0x80,
                                                                                         0xAA, 0x55, 0xAA, 0x55, 0x01};

JunkInTheTrunkPacket junk_in_the_trunk_packet {junk_in_the_trunk_packet_field_1, junk_in_the_trunk_packet_field_2,
                                               junk_in_the_trunk_packet_field_3, junk_in_the_trunk_packet_field_4};

// This is going straight over the top
// Expect padding between the 4th and fifth fields (second unit bit and NestedPacket)
using NestedPacket = packet_t<Bit, BitField<15>>;
NestedPacket::value_type_t<0U> nested_packet_nested_field_1 {false};
NestedPacket::value_type_t<1U> nested_packet_nested_field_2 {0x5A5A};

using NestedPacketPacket = packet_t<BitField<4>, BitField<11>, Bit, Bit, NestedPacket, int8_t>;
NestedPacketPacket::value_type_t<0U> nested_packet_packet_field_1 {0x0F};
NestedPacketPacket::value_type_t<1U> nested_packet_packet_field_2 {0x5A4};
NestedPacketPacket::value_type_t<2U> nested_packet_packet_field_3 {true};
NestedPacketPacket::value_type_t<3U> nested_packet_packet_field_4 {false};
NestedPacketPacket::value_type_t<4U> nested_packet_packet_field_5 {nested_packet_nested_field_1,
                                                                   nested_packet_nested_field_2};
NestedPacketPacket::value_type_t<5U> nested_packet_packet_field_6 {-42};
/*
    The Packet structure declared above would be represented as follows:

                                    bytes
                                    0 | 1
      1   1   1   1   0   0   1   0   |   0   1   0   1   1   0   1   1
     ---------------------------------|---------------------------------
    |       4       |                 ^       2                     | 3 |
           4 bits                           11 bits                  1 bit
           (0x0F)                           (0x5A4)                  (0x01)
                                    bytes
                                    2 | 3
      0   0   0   0   0   0   0   0   |   0   0   1   0   1   1   0   1
     ---------------------------------|---------------------------------
    | 4 |         padding             |   5 |           6
    1 bit         7 bits                1 bit         15 bits
    (0x00)                              (0x00)        (0x5A5A)
                                    bytes
                                    4 | 5
      0   0   1   0   1   1   0   1   |   0   1   1   0   1   0   1   1
     ---------------------------------|---------------------------------
                  6 cont.             |                 7
                                                       8 bits
                                                       (0xD6)

    The resultant byte span should equal: {0x4F, 0xDA, 0x00, 0xB4, 0xB4, 0xD6}
*/
constexpr size_t kNestedPacketPacketExpectedSizeBits {48U};
constexpr size_t kNestedPacketPacketExpectedSizeBytes {6U};
uint8_t nested_packet_packet_expected_bytes[kNestedPacketPacketExpectedSizeBytes] {0x4F, 0xDA, 0x00, 0xB4, 0xB4, 0xD6};

NestedPacketPacket nested_packet_packet {nested_packet_packet_field_1, nested_packet_packet_field_2,
                                         nested_packet_packet_field_3, nested_packet_packet_field_4,
                                         nested_packet_packet_field_5, nested_packet_packet_field_6};

// And this is just downright insane
using DoubleNestedPacket = packet_t<uint8_t, NestedPacket>;
DoubleNestedPacket::value_type_t<0U> double_nested_packet_field_1 {0xA5};
NestedPacket::value_type_t<0U>       double_nested_packet_nested_packet_field_1 {false};
NestedPacket::value_type_t<1U>       double_nested_packet_nested_packet_field_2 {0x5A5A};

NestedPacket::value_type_t<0U> double_nested_packet_nested_field_1 {true};
NestedPacket::value_type_t<1U> double_nested_packet_nested_field_2 {0x25A5};

using DoubleNestedPacketPacket = packet_t<uint32_t, DoubleNestedPacket, NestedPacket, BitField<24>>;
DoubleNestedPacketPacket::value_type_t<0U> double_nested_packet_packet_field_1 {0x700FF00E};
DoubleNestedPacketPacket::value_type_t<1U> double_nested_packet_packet_field_2 {
    double_nested_packet_field_1,
    NestedPacket {double_nested_packet_nested_packet_field_1, double_nested_packet_nested_packet_field_2}};
DoubleNestedPacketPacket::value_type_t<2U> double_nested_packet_packet_field_3 {double_nested_packet_nested_field_1,
                                                                                double_nested_packet_nested_field_2};
DoubleNestedPacketPacket::value_type_t<3U> double_nested_packet_packet_field_4 {0xFFA5A5};
/*
    The Packet structure declared above would be represented as follows:

                                    bytes
                                    0 | 1
      0   1   1   1   0   0   0   0   |   0   0   0   0   1   1   1   1
     ---------------------------------|---------------------------------
    |                1                ^
                   32 bits
                 (0x700FF00E)

                                    bytes
                                    2 | 3
      1   1   1   1   0   0   0   0   |   0   0   0   0   1   1   1   0
     ---------------------------------|---------------------------------
                  1 cont.             ^                                 |



                                    bytes
                                    4 | 5
      1   0   1   0   0   1   0   1   |   0   0   1   0   1   1   0   1
     ---------------------------------|---------------------------------
                    2                 |   3 |             4
                   8 bits              1 bit            15 bits
                   (0xA5)              (0x00)           (0x5A5A)

                                    bytes
                                    6 | 7
      0   0   1   0   1   1   0   1   |   1   1   0   1   0   0   1   0
     ---------------------------------|---------------------------------
                  4 cont.             |   5 |             6
                                       1 bit            15 bits
                                       (0x01)           (0x25A5)

                                    bytes
                                    8 | 9
      1   1   0   1   0   0   1   0   |   1   0   1   0   0   1   0   1
     ---------------------------------|---------------------------------
                  6 cont.             |                 7
                                                      24 bits
                                                     (0xFFA5A5)

                                    bytes
                                   10 | 11
      1   0   1   0   0   1   0   1   |   1   1   1   1   1   1   1   1
     ---------------------------------|---------------------------------
                  7 cont.             ^                                 |

    The resultant byte span should equal: {0x0E, 0xF0, 0x0F, 0x70, 0xA5, 0xB4, 0xB4, 0x4B, 0x4B, 0xA5, 0xA5, 0xFF}
*/
constexpr size_t kDoubleNestedPacketPacketExpectedSizeBits {96U};
constexpr size_t kDoubleNestedPacketPacketExpectedSizeBytes {12U};
uint8_t double_nested_packet_packet_expected_bytes[kDoubleNestedPacketPacketExpectedSizeBytes] {0x0E, 0xF0, 0x0F, 0x70,
                                                                                                0xA5, 0xB4, 0xB4, 0x4B,
                                                                                                0x4B, 0xA5, 0xA5, 0xFF};

DoubleNestedPacketPacket double_nested_packet_packet {double_nested_packet_packet_field_1,
                                                      double_nested_packet_packet_field_2,
                                                      double_nested_packet_packet_field_3,
                                                      double_nested_packet_packet_field_4};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Packet tests ////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Test that values stored by a Packet are stripped of their reference and CV qualifications.
 *
 */
TEST(Packet, stored_types_decay_qualifications)
{
    using TestField1 = volatile const int;
    using TestField2 = std::add_lvalue_reference_t<double>;
    using TestField3 = std::add_const_t<std::add_lvalue_reference_t<uint64_t>>;

    using TestPacket = packet_t<TestField1, TestField2, TestField3>;

    ::testing::StaticAssertTypeEq<std::decay_t<TestField1>, TestPacket::value_type_t<0U>>();
    ::testing::StaticAssertTypeEq<std::decay_t<TestField2>, TestPacket::value_type_t<1U>>();
    ::testing::StaticAssertTypeEq<std::decay_t<TestField3>, TestPacket::value_type_t<2U>>();
}

/**
 * Test that calculated Packet sizes match expected amounts
 *
 */
TEST(Packet, accumulated_size)
{
    // FunSizePacket
    constexpr size_t kFunSizePacketSizeBits {FunSizePacket::kSizeBits};
    constexpr size_t kFunSizePacketSizeBytes {FunSizePacket::kSizeBytes};
    EXPECT_EQ(kFunSizePacketExpectedSizeBits, kFunSizePacketSizeBits);
    EXPECT_EQ(kFunSizePacketExpectedSizeBytes, kFunSizePacketSizeBytes);

    // LooselyPackedPacket
    constexpr size_t kLooselyPackedPacketSizeBits {LooselyPackedPacket::kSizeBits};
    constexpr size_t kLooselyPackedPacketSizeBytes {LooselyPackedPacket::kSizeBytes};
    EXPECT_EQ(kLooselyPackedPacketExpectedSizeBits, kLooselyPackedPacketSizeBits);
    EXPECT_EQ(kLooselyPackedPacketExpectedSizeBytes, kLooselyPackedPacketSizeBytes);

    // TightlyPackedPacket
    constexpr size_t kTightlyPackedPacketSizeBits {TightlyPackedPacket::kSizeBits};
    constexpr size_t kTightlyPackedPacketSizeBytes {TightlyPackedPacket::kSizeBytes};
    EXPECT_EQ(kTightlyPackedPacketExpectedSizeBits, kTightlyPackedPacketSizeBits);
    EXPECT_EQ(kTightlyPackedPacketExpectedSizeBytes, kTightlyPackedPacketSizeBytes);

    // JunkInTheTrunkPacket
    constexpr size_t kJunkInTheTrunkPacketSizeBits {JunkInTheTrunkPacket::kSizeBits};
    constexpr size_t kJunkInTheTrunkPacketSizeBytes {JunkInTheTrunkPacket::kSizeBytes};
    EXPECT_EQ(kJunkInTheTrunkPacketExpectedSizeBits, kJunkInTheTrunkPacketSizeBits);
    EXPECT_EQ(kJunkInTheTrunkPacketExpectedSizeBytes, kJunkInTheTrunkPacketSizeBytes);

    // NestedPacketPacket
    constexpr size_t kNestedPacketPacketSizeBits {NestedPacketPacket::kSizeBits};
    constexpr size_t kNestedPacketPacketSizeBytes {NestedPacketPacket::kSizeBytes};
    EXPECT_EQ(kNestedPacketPacketExpectedSizeBits, kNestedPacketPacketSizeBits);
    EXPECT_EQ(kNestedPacketPacketExpectedSizeBytes, kNestedPacketPacketSizeBytes);

    // DoubleNestedPacketPacket
    constexpr size_t kDoubleNestedPacketPacketSizeBits {DoubleNestedPacketPacket::kSizeBits};
    constexpr size_t kDoubleNestedPacketPacketSizeBytes {DoubleNestedPacketPacket::kSizeBytes};
    EXPECT_EQ(kDoubleNestedPacketPacketExpectedSizeBits, kDoubleNestedPacketPacketSizeBits);
    EXPECT_EQ(kDoubleNestedPacketPacketExpectedSizeBytes, kDoubleNestedPacketPacketSizeBytes);
}

/**
 * Test that Packet fields can be initialized with arguments on construction
 *
 */
TEST(Packet, initializing_constructor)
{
    // FunSizePacket
    EXPECT_EQ(true, packet_field_value<0U>(fun_size_packet));
    EXPECT_EQ(false, packet_field_value<1U>(fun_size_packet));
    EXPECT_EQ(true, packet_field_value<2U>(fun_size_packet));
    EXPECT_EQ(false, packet_field_value<3U>(fun_size_packet));
    EXPECT_EQ(true, packet_field_value<4U>(fun_size_packet));

    // LooselyPackedPacket
    EXPECT_EQ(loosely_packed_packet_field_1, packet_field_value<0U>(loosely_packed_packet));
    EXPECT_EQ(loosely_packed_packet_field_2, packet_field_value<1U>(loosely_packed_packet));
    EXPECT_EQ(loosely_packed_packet_field_3, packet_field_value<2U>(loosely_packed_packet));
    EXPECT_EQ(loosely_packed_packet_field_4, packet_field_value<3U>(loosely_packed_packet));
    EXPECT_EQ(loosely_packed_packet_field_5, packet_field_value<4U>(loosely_packed_packet));

    // TightlyPackedPacket
    EXPECT_EQ(tightly_packed_packet_field_1, packet_field_value<0U>(tightly_packed_packet));
    EXPECT_EQ(tightly_packed_packet_field_2, packet_field_value<1U>(tightly_packed_packet));
    EXPECT_EQ(tightly_packed_packet_field_3, packet_field_value<2U>(tightly_packed_packet));
    EXPECT_EQ(tightly_packed_packet_field_4, packet_field_value<3U>(tightly_packed_packet));
    EXPECT_EQ(tightly_packed_packet_field_5, packet_field_value<4U>(tightly_packed_packet));
    EXPECT_EQ(tightly_packed_packet_field_6, packet_field_value<5U>(tightly_packed_packet));

    // JunkInTheTrunkPacket
    EXPECT_EQ(junk_in_the_trunk_packet_field_1, packet_field_value<0U>(junk_in_the_trunk_packet));
    EXPECT_EQ(junk_in_the_trunk_packet_field_2, packet_field_value<1U>(junk_in_the_trunk_packet));
    EXPECT_EQ(junk_in_the_trunk_packet_field_3, packet_field_value<2U>(junk_in_the_trunk_packet));
    EXPECT_EQ(junk_in_the_trunk_packet_field_4, packet_field_value<3U>(junk_in_the_trunk_packet));

    // NestedPacketPacket
    EXPECT_EQ(nested_packet_packet_field_1, packet_field_value<0U>(nested_packet_packet));
    EXPECT_EQ(nested_packet_packet_field_2, packet_field_value<1U>(nested_packet_packet));
    EXPECT_EQ(nested_packet_packet_field_3, packet_field_value<2U>(nested_packet_packet));
    EXPECT_EQ(nested_packet_packet_field_4, packet_field_value<3U>(nested_packet_packet));
    // Field 5 of NestedPacketPacket is a Packet type and must be further extracted. Testing performed below.
    EXPECT_EQ(nested_packet_packet_field_6, packet_field_value<5U>(nested_packet_packet));

    NestedPacket fetched_nested_packet {packet_field_value<4U>(nested_packet_packet)};
    EXPECT_EQ(nested_packet_nested_field_1, packet_field_value<0U>(fetched_nested_packet));
    EXPECT_EQ(nested_packet_nested_field_2, packet_field_value<1U>(fetched_nested_packet));

    // DoubleNestedPacketPacket
    EXPECT_EQ(double_nested_packet_packet_field_1, packet_field_value<0U>(double_nested_packet_packet));
    // Field 2 of DoubleNestedPacketPacket is a Packet type and must be further extracted. Testing performed below.
    // Field 3 of DoubleNestedPacketPacket is a Packet type and must be further extracted. Testing performed below.
    EXPECT_EQ(double_nested_packet_packet_field_4, packet_field_value<3U>(double_nested_packet_packet));

    DoubleNestedPacket fetched_double_nested_packet {packet_field_value<1U>(double_nested_packet_packet)};
    NestedPacket       fetched_double_nested_packet_field_2 {packet_field_value<1U>(fetched_double_nested_packet)};
    EXPECT_EQ(double_nested_packet_field_1, packet_field_value<0U>(fetched_double_nested_packet));
    EXPECT_EQ(double_nested_packet_nested_packet_field_1, packet_field_value<0U>(fetched_double_nested_packet_field_2));
    EXPECT_EQ(double_nested_packet_nested_packet_field_2, packet_field_value<1U>(fetched_double_nested_packet_field_2));

    fetched_nested_packet = packet_field_value<2U>(double_nested_packet_packet);
    EXPECT_EQ(double_nested_packet_nested_field_1, packet_field_value<0U>(fetched_nested_packet));
    EXPECT_EQ(double_nested_packet_nested_field_2, packet_field_value<1U>(fetched_nested_packet));
}

/**
 * Test that values stored by a Packet's fields are sequentially encoded in to a byte buffer
 *
 */
TEST(Packet, encode)
{
    // FunSizePacket
    size_t        fun_size_packet_bits_encoded {0u};
    uint8_t       fun_size_packet_encode_buffer[FunSizePacket::kSizeBytes];
    Span<uint8_t> fun_size_packet_encode_span(fun_size_packet_encode_buffer, FunSizePacket::kSizeBytes);
    std::memset(fun_size_packet_encode_buffer, 0U, FunSizePacket::kSizeBytes);
    auto result {encode(fun_size_packet, fun_size_packet_encode_span, fun_size_packet_bits_encoded)};
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kFunSizePacketExpectedSizeBits, fun_size_packet_bits_encoded);

    Span<uint8_t> fun_size_packet_expected_bytes_span {fun_size_packet_expected_bytes, kFunSizePacketExpectedSizeBytes};
    EXPECT_TRUE(byte_spans_match(fun_size_packet_expected_bytes_span, fun_size_packet_encode_span));

    // LooselyPackedPacket
    size_t  loosely_packed_packet_bits_encoded {0u};
    uint8_t loosely_packed_packet_encode_buffer[LooselyPackedPacket::kSizeBytes];
    Span<uint8_t> loosely_packed_packet_encode_span(loosely_packed_packet_encode_buffer, LooselyPackedPacket::kSizeBytes);
    std::memset(loosely_packed_packet_encode_buffer, 0U, LooselyPackedPacket::kSizeBytes);
    result = encode(loosely_packed_packet, loosely_packed_packet_encode_span, loosely_packed_packet_bits_encoded);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kLooselyPackedPacketExpectedSizeBits, loosely_packed_packet_bits_encoded);

    Span<uint8_t> loosely_packed_packet_expected_bytes_span {loosely_packed_packet_expected_bytes,
                                                             kLooselyPackedPacketExpectedSizeBytes};
    EXPECT_TRUE(byte_spans_match(loosely_packed_packet_expected_bytes_span, loosely_packed_packet_encode_span));

    // TightlyPackedPacket
    size_t  tightly_packed_packet_bits_encoded {0u};
    uint8_t tightly_packed_packet_encode_buffer[TightlyPackedPacket::kSizeBytes];
    Span<uint8_t> tightly_packed_packet_encode_span(tightly_packed_packet_encode_buffer, TightlyPackedPacket::kSizeBytes);
    std::memset(tightly_packed_packet_encode_buffer, 0U, TightlyPackedPacket::kSizeBytes);
    result = encode(tightly_packed_packet, tightly_packed_packet_encode_span, tightly_packed_packet_bits_encoded);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kTightlyPackedPacketExpectedSizeBits, tightly_packed_packet_bits_encoded);

    Span<uint8_t> tightly_packed_packet_expected_bytes_span {tightly_packed_packet_expected_bytes,
                                                             kTightlyPackedPacketExpectedSizeBytes};
    EXPECT_TRUE(byte_spans_match(tightly_packed_packet_expected_bytes_span, tightly_packed_packet_encode_span));

    // JunkInTheTrunkPacket
    size_t        junk_in_the_trunk_packet_bits_encoded {0u};
    uint8_t       junk_in_the_trunk_packet_encode_buffer[JunkInTheTrunkPacket::kSizeBytes];
    Span<uint8_t> junk_in_the_trunk_packet_encode_span(junk_in_the_trunk_packet_encode_buffer,
                                                       JunkInTheTrunkPacket::kSizeBytes);
    std::memset(junk_in_the_trunk_packet_encode_buffer, 0U, JunkInTheTrunkPacket::kSizeBytes);
    result = encode(junk_in_the_trunk_packet, junk_in_the_trunk_packet_encode_span, junk_in_the_trunk_packet_bits_encoded);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kJunkInTheTrunkPacketExpectedSizeBits, junk_in_the_trunk_packet_bits_encoded);

    Span<uint8_t> junk_in_the_trunk_packet_expected_bytes_span {junk_in_the_trunk_packet_expected_bytes,
                                                                kJunkInTheTrunkPacketExpectedSizeBytes};
    EXPECT_TRUE(byte_spans_match(junk_in_the_trunk_packet_expected_bytes_span, junk_in_the_trunk_packet_encode_span));

    // NestedPacketPacket
    size_t        nested_packet_packet_bits_encoded {0u};
    uint8_t       nested_packet_packet_encode_buffer[NestedPacketPacket::kSizeBytes];
    Span<uint8_t> nested_packet_packet_encode_span(nested_packet_packet_encode_buffer, NestedPacketPacket::kSizeBytes);
    std::memset(nested_packet_packet_encode_buffer, 0U, NestedPacketPacket::kSizeBytes);
    result = encode(nested_packet_packet, nested_packet_packet_encode_span, nested_packet_packet_bits_encoded);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kNestedPacketPacketExpectedSizeBits, nested_packet_packet_bits_encoded);

    Span<uint8_t> nested_packet_packet_expected_bytes_span {nested_packet_packet_expected_bytes,
                                                            kNestedPacketPacketExpectedSizeBytes};
    EXPECT_TRUE(byte_spans_match(nested_packet_packet_expected_bytes_span, nested_packet_packet_encode_span));

    // DoubleNestedPacketPacket
    size_t        double_nested_packet_packet_bits_encoded {0u};
    uint8_t       double_nested_packet_packet_encode_buffer[DoubleNestedPacketPacket::kSizeBytes];
    Span<uint8_t> double_nested_packet_packet_encode_span(double_nested_packet_packet_encode_buffer,
                                                          DoubleNestedPacketPacket::kSizeBytes);
    std::memset(double_nested_packet_packet_encode_buffer, 0U, DoubleNestedPacketPacket::kSizeBytes);
    result = encode(double_nested_packet_packet, double_nested_packet_packet_encode_span,
                    double_nested_packet_packet_bits_encoded);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kDoubleNestedPacketPacketExpectedSizeBits, double_nested_packet_packet_bits_encoded);

    Span<uint8_t> double_nested_packet_packet_expected_bytes_span {double_nested_packet_packet_expected_bytes,
                                                                   kDoubleNestedPacketPacketExpectedSizeBytes};
    EXPECT_TRUE(byte_spans_match(double_nested_packet_packet_expected_bytes_span, double_nested_packet_packet_encode_span));
}

/**
 * @brief Tests whether two individual instances of the same Packet specialization hold identical fields
 *
 * @tparam FieldT Type parameter list representing the values to be held as fields by the Packet. Fields are
 * stored in the order they are presented and are accessible through their positional index, starting at 0. Wrapped
 * types such as Field, BitField, and ConstBitField are stored as-is while unwrapped types will be wrapped by Field.
 * @param p1 Packet 1
 * @param p2 Packet 2
 * @return ::testing::AssertionResult
 */
template<typename... FieldT>
::testing::AssertionResult packets_match(Packet<FieldT...> const& p1, Packet<FieldT...> const& p2);

/**
 * @brief Match two values
 *
 * @tparam T Value type to match
 * @param v1 Value 1
 * @param v2 Value 2
 * @return ::testing::AssertionResult
 */
template<typename T>
::testing::AssertionResult match_values(T const& v1, T const& v2)
{
    if (v1 != v2)
        return ::testing::AssertionFailure() << "\r\nValue mismatch: " << v1 << " != " << v2;
    return ::testing::AssertionSuccess();
}

/**
 * @brief match_value specialization for Packet types
 *
 * @tparam FieldT Type parameter list representing the values to be held as fields by the Packet. Fields are
 * stored in the order they are presented and are accessible through their positional index, starting at 0. Wrapped
 * types such as Field, BitField, and ConstBitField are stored as-is while unwrapped types will be wrapped by Field.
 * @param v1 Packet 1
 * @param v2 Packet 2
 * @return ::testing::AssertionResult
 */
template<typename... FieldT>
::testing::AssertionResult match_values(Packet<FieldT...> const& v1, Packet<FieldT...> const& v2)
{
    return packets_match(v1, v2) << "\r\nfield is in nested packet";
}

/**
 * @brief Sequentially compares the fields held by two individual instances of the same Packet specialization
 *
 * @tparam IndexV Current field position
 * @tparam EndV Endpoint for recursion
 * @tparam FieldT Type parameter list representing the values to be held as fields by the Packet. Fields are
 * stored in the order they are presented and are accessible through their positional index, starting at 0. Wrapped
 * types such as Field, BitField, and ConstBitField are stored as-is while unwrapped types will be wrapped by Field.
 */
template<size_t IndexV, size_t EndV, typename... FieldT>
struct match_fields_recursive
{
    static_assert((EndV <= sizeof...(FieldT)), "`EndV` must not be greater than field count");
    static_assert(IndexV < EndV, "`IndexV` must be within `EndV` bounds");

    static ::testing::AssertionResult Do(Packet<FieldT...> const& p1, Packet<FieldT...> const& p2)
    {
        auto& value1 {packet_field_value<IndexV>(p1)};
        auto& value2 {packet_field_value<IndexV>(p2)};

        ::testing::AssertionResult result {match_values(value1, value2)};
        if (!result)
            return result << "\r\nat position " << IndexV;

        return match_fields_recursive<(IndexV + 1U), EndV, FieldT...>::Do(p1, p2);
    }
};

/**
 * @brief Base case for recursive comparison of Packet fieldse; `IndexV == EndV`
 *
 * @tparam IndexV
 * @tparam FieldT
 */
template<size_t IndexV, typename... FieldT>
struct match_fields_recursive<IndexV, IndexV, FieldT...>
{
    static ::testing::AssertionResult Do(Packet<FieldT...> const& p1, Packet<FieldT...> const& p2)
    {
        // At end of packet, do nothing
        static_cast<void>(p1); // Avoid unused warning
        static_cast<void>(p2); // Avoid unused warning
        return ::testing::AssertionSuccess();
    }
};

template<typename... FieldT>
::testing::AssertionResult packets_match(Packet<FieldT...> const& p1, Packet<FieldT...> const& p2)
{
    return match_fields_recursive<0U, Packet<FieldT...>::kNumFields, FieldT...>::Do(p1, p2);
}

/**
 * Test that values stored by a Packet's fields are sequentially decoded from a byte buffer
 *
 */
TEST(Packet, decode)
{
    // FunsizePacket
    FunSizePacket fun_size_packet_decoded;
    Span<uint8_t const> fun_size_packet_encoded_bytes_span {fun_size_packet_expected_bytes, kFunSizePacketExpectedSizeBytes};
    size_t fun_size_packet_bits_decoded {0U};
    auto   result {decode(fun_size_packet_encoded_bytes_span, fun_size_packet_bits_decoded, fun_size_packet_decoded)};

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kFunSizePacketExpectedSizeBits, fun_size_packet_bits_decoded);
    EXPECT_TRUE(packets_match(fun_size_packet, fun_size_packet_decoded));

    // LooselyPackedPacket
    LooselyPackedPacket loosely_packed_packet_decoded;
    Span<uint8_t const> loosely_packed_packet_encoded_bytes_span {loosely_packed_packet_expected_bytes,
                                                                  kLooselyPackedPacketExpectedSizeBytes};
    size_t              loosely_packed_packet_encoded_bits_decoded {0U};
    result = decode(loosely_packed_packet_encoded_bytes_span, loosely_packed_packet_encoded_bits_decoded,
                    loosely_packed_packet_decoded);

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kLooselyPackedPacketExpectedSizeBits, loosely_packed_packet_encoded_bits_decoded);
    EXPECT_TRUE(packets_match(loosely_packed_packet, loosely_packed_packet_decoded));

    // TightlyPackedPacket
    TightlyPackedPacket tightly_packed_packet_decoded;
    Span<uint8_t const> tightly_packed_packet_encoded_bytes_span {tightly_packed_packet_expected_bytes,
                                                                  kTightlyPackedPacketExpectedSizeBytes};
    size_t              tightly_packed_packet_encoded_bits_decoded {0U};
    result = decode(tightly_packed_packet_encoded_bytes_span, tightly_packed_packet_encoded_bits_decoded,
                    tightly_packed_packet_decoded);

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kTightlyPackedPacketExpectedSizeBits, tightly_packed_packet_encoded_bits_decoded);
    EXPECT_TRUE(packets_match(tightly_packed_packet, tightly_packed_packet_decoded));

    // JunkInTheTrunkPacket
    JunkInTheTrunkPacket junk_in_the_trunk_packet_decoded;
    Span<uint8_t const>  junk_in_the_trunk_packet_encoded_bytes_span {junk_in_the_trunk_packet_expected_bytes,
                                                                     kJunkInTheTrunkPacketExpectedSizeBytes};
    size_t               junk_in_the_trunk_packet_encoded_bits_decoded {0U};
    result = decode(junk_in_the_trunk_packet_encoded_bytes_span, junk_in_the_trunk_packet_encoded_bits_decoded,
                    junk_in_the_trunk_packet_decoded);

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kJunkInTheTrunkPacketExpectedSizeBits, junk_in_the_trunk_packet_encoded_bits_decoded);
    EXPECT_TRUE(packets_match(junk_in_the_trunk_packet, junk_in_the_trunk_packet_decoded));

    // NestedPacketPacket
    NestedPacketPacket  nested_packet_packet_decoded;
    Span<uint8_t const> nested_packet_packet_encoded_bytes_span {nested_packet_packet_expected_bytes,
                                                                 kNestedPacketPacketExpectedSizeBytes};
    size_t              nested_packet_packet_encoded_bits_decoded {0U};
    result = decode(nested_packet_packet_encoded_bytes_span, nested_packet_packet_encoded_bits_decoded,
                    nested_packet_packet_decoded);

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kNestedPacketPacketExpectedSizeBits, nested_packet_packet_encoded_bits_decoded);
    EXPECT_TRUE(packets_match(nested_packet_packet, nested_packet_packet_decoded));

    // DoubleNestedPacketPacket
    DoubleNestedPacketPacket double_nested_packet_packet_decoded;
    Span<uint8_t const>      double_nested_packet_packet_encoded_bytes_span {double_nested_packet_packet_expected_bytes,
                                                                        kDoubleNestedPacketPacketExpectedSizeBytes};
    size_t                   double_nested_packet_packet_encoded_bits_decoded {0U};
    result = decode(double_nested_packet_packet_encoded_bytes_span, double_nested_packet_packet_encoded_bits_decoded,
                    double_nested_packet_packet_decoded);

    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(kDoubleNestedPacketPacketExpectedSizeBits, double_nested_packet_packet_encoded_bits_decoded);
    EXPECT_TRUE(packets_match(double_nested_packet_packet, double_nested_packet_packet_decoded));
}

// Encode Packet comprised of Fields

// Encode Packet comprised of BitFields

// Encode Packet comprised of a combination
