#pragma once

#include "Footprint.hpp"

#include <type_traits>

namespace shmit
{
namespace data
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Namespace metafunction declarations             ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**!
 * @brief Returns the smallest signed type given a number of bits to contain
 *
 * @tparam BitSizeV Minimum number of bits to contain
 */
template<size_t BitSizeV>
struct smallest_signed;

/**!
 * @brief Returns the smallest unsigned type given a number of bits to contain
 *
 * @tparam BitSizeV Minimum number of bits to contain
 */
template<size_t BitSizeV>
struct smallest_unsigned;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Namespace metafunction definitions in alphabetical order                ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<size_t BitSizeV>
struct smallest_signed
{
    static_assert(BitSizeV <= 64U, "`BitSizeV` must be <= 64");

    using type =
        std::conditional_t<check_if_fits_v<BitSizeV, int8_t>, int8_t,
                           std::conditional_t<check_if_fits_v<BitSizeV, int16_t>, int16_t,
                                              std::conditional_t<check_if_fits_v<BitSizeV, int32_t>, int32_t, int64_t>>>;
};

/**!
 * @brief Convenience alias to access the returned type of smallest_signed
 *
 * @tparam BitSizeV Minimum number of bits to contain
 */
template<size_t BitSizeV>
using smallest_signed_t = typename smallest_signed<BitSizeV>::type;

template<size_t BitSizeV>
struct smallest_unsigned
{
    static_assert(BitSizeV <= 64U, "`BitSizeV` must be <= 64");

    using type =
        std::conditional_t<check_if_fits_v<BitSizeV, uint8_t>, uint8_t,
                           std::conditional_t<check_if_fits_v<BitSizeV, uint16_t>, uint16_t,
                                              std::conditional_t<check_if_fits_v<BitSizeV, uint32_t>, uint32_t, uint64_t>>>;
};

/**!
 * @brief Convenience alias to access the returned type of smallest_unsigned
 *
 * @tparam BitSizeV Minimum number of bits to contain
 */
template<size_t BitSizeV>
using smallest_unsigned_t = typename smallest_unsigned<BitSizeV>::type;

} // namespace data
} // namespace shmit