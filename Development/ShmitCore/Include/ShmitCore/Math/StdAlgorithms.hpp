#include <ShmitCore/StdTypes.hpp>

#include <type_traits>

namespace shmit
{
namespace math
{

template<typename T, typename R = T>
constexpr static R abs(T value) noexcept
{
    static_assert(std::is_arithmetic<T>::value, "T must be fundamental arithmetic type");
    static_assert(std::is_arithmetic<R>::value, "T must be fundamental arithmetic type");

    if (std::is_signed<T>::value)
    {
        T result = value;

        if (value < 0)
            result *= -1;

        return static_cast<R>(result);
    }
    else
        return static_cast<R>(value);
}

template<typename T>
constexpr static size_t gcd(T a, T b) noexcept
{
    static_assert(std::is_arithmetic<T>::value, "T must be fundamental arithmetic type");

    T remainder {abs(a % b)};
    while (remainder > 0)
    {
        a         = b;
        b         = remainder;
        remainder = abs(a % b);
    }

    return b;
}

} // namespace math
} // namespace shmit