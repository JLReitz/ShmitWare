#pragma once

#include "Core/Math/Memory.hpp"
#include "Core/StdTypes.hpp"

#include <climits>

namespace shmit
{
namespace system
{

/**
 * @brief
 *
 */
struct Fundamental
{
    constexpr static size_t kUnitSizeBits {CHAR_BIT};
    constexpr static size_t kUnitSizeBytes {math::bytes_to_contain(CHAR_BIT)};

    template<typename T>
    struct type_footprint
    {
        constexpr static size_t value {sizeof(T)};
    };

    template<typename T>
    constexpr static size_t type_footprint_v {type_footprint<T>::value};
};

} // namespace system
} // namespace shmit