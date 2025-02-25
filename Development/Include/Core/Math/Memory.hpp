#pragma once

#include "Core/StdTypes.hpp"

namespace shmit
{
namespace math
{

constexpr static size_t bits_to_contain(size_t num_bytes) noexcept
{
    /*
       Do some sneaky bitwise operations to emulate modulo and division.

       Because 2^3 = 8:
        - "n * 8" can be done as "n << 3"

       This helps avoid complex math operations where possible
    */
    constexpr size_t kMultiplyBy8Shift {3U};
    return num_bytes << kMultiplyBy8Shift;
}

constexpr static size_t bytes_to_contain(size_t num_bits) noexcept
{
    /*
       Do some sneaky bitwise operations to emulate modulo and division.

       Because 2^3 = 8:
        - "n / 8" can be done as "n >> 3"
        - "n % 8" can be done as "n & b00000111"

       This helps avoid complex math operations where possible
    */
    constexpr size_t kDivideBy8Shift {3U};
    constexpr size_t kModulo8Mask {0x7};
    bool const       kSizeOverflowsBoundary {(num_bits & kModulo8Mask) != 0};
    size_t const     kBaseSizeBytes {num_bits >> kDivideBy8Shift};

    return kBaseSizeBytes + (kSizeOverflowsBoundary ? 1U : 0U);
}

constexpr static size_t next_boundary_bit_pos(size_t start_bit_pos) noexcept
{
    size_t next_byte_offset {bytes_to_contain(start_bit_pos)};
    return bits_to_contain(next_byte_offset);
}

} // namespace math
} // namespace shmit