#pragma once

#include "Core/Math/Memory.hpp"
#include "Core/StdTypes.hpp"
#include "Core/System/Fundamental.hpp"

#include <type_traits>

namespace shmit
{
namespace data
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Namespace metafunction declarations             ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**!
 * @brief Calculate the size of a type in bits
 *
 * @tparam T Any type
 */
template<typename T>
struct footprint_size_bits;

/**!
 * @brief Calculate the size of a type in bytes
 *
 * @tparam T Any type
 */
template<typename T>
struct footprint_size_bytes;

/**!
 * @brief Checks if a type fits within a number of bits
 *
 * @tparam BitSizeV Number of bits
 * @tparam T Type to check
 */
template<size_t BitSizeV, typename T>
struct check_if_fits;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Namespace metafunction definitions              ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct footprint_size_bits
{
    constexpr static size_t value {system::Fundamental::type_footprint<T>::value * system::Fundamental::kUnitSizeBits};
};

/**!
 * @brief Convenience alias to access the returned value of footprint_size_bits
 *
 * @tparam T Any type
 */
template<typename T>
constexpr static size_t footprint_size_bits_v {footprint_size_bits<T>::value};

template<typename T>
struct footprint_size_bytes
{
    constexpr static size_t value {math::bytes_to_contain(footprint_size_bits<T>::value)};
};

/**!
 * @brief Convenience alias to access the returned value of footprint_size_bytes
 *
 * @tparam T Any type
 */
template<typename T>
constexpr static size_t footprint_size_bytes_v {footprint_size_bytes<T>::value};

template<size_t BitSizeV, typename T>
struct check_if_fits
{
    using type = std::conditional_t<(BitSizeV <= footprint_size_bits<T>::value), std::true_type, std::false_type>;

    constexpr static bool value {type::value};
};

/**!
 * @brief Convenience alias to access the returned type of check_if_fits
 *
 * @tparam BitSizeV Number of bits
 * @tparam T Type to check
 */
template<size_t BitSizeV, typename T>
using check_if_fits_t = typename check_if_fits<BitSizeV, T>::type;

/**!
 * @brief Convenience alias to access the returned value of check_if_fits
 *
 * @tparam BitSizeV Number of bits
 * @tparam T Type to check
 */
template<size_t BitSizeV, typename T>
constexpr static bool check_if_fits_v {check_if_fits<BitSizeV, T>::value};

} // namespace data
} // namespace shmit