#pragma once

#include <ShmitCore/Span.hpp>
#include <ShmitCore/StdTypes.hpp>

#include <gtest/gtest.h>

#include <sstream>

/**!
 * @brief Print entries from a byte span in hex
 *
 * @param span Byte span
 * @return std::string String containing the hex printout
 */
static std::string print_bytes_hex(shmit::Span<uint8_t> span)
{
    std::stringstream ss;
    ss << std::hex;
    for (uint8_t const& byte : span)
        ss << static_cast<int>(byte) << " ";
    return ss.str();
}

/**!
 * @brief Test two byte spans, comparing length and contents
 *
 * @param span1 One byte span
 * @param span2 Other byte span
 * @return ::testing::AssertionResult
 */
static ::testing::AssertionResult byte_spans_match(shmit::Span<uint8_t> span1, shmit::Span<uint8_t> span2)
{
    // Check span counts, if not the same return early
    if (span1.count() != span2.count())
        return ::testing::AssertionFailure() << "Byte spans do not match in length\r\nSpan 1 (" << span1.count()
                                             << " bytes): " << print_bytes_hex(span1) << "\r\nSpan 2 (" << span2.count()
                                             << " bytes): " << print_bytes_hex(span2);

    for (size_t i = 0; i < span1.count(); i++)
    {
        if (span1[i] != span2[i])
        {
            return ::testing::AssertionFailure()
                   << "Byte mismatch at offset " << i << "\r\nSpan 1: " << print_bytes_hex(span1)
                   << "\r\nSpan2: " << print_bytes_hex(span2);
        }
    }

    return ::testing::AssertionSuccess();
}