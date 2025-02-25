#pragma once

#include "Core/StdTypes.hpp"

#include <string_view>
#include <type_traits>

#define string_constant(str) decltype(str##_ssc)

namespace shmit
{

template<typename CharT, CharT... Chars>
struct StringConstant
{
private:
    constexpr static size_t kDataSize {sizeof...(Chars) + 1};
    constexpr static CharT  kData[kDataSize] {Chars..., '\0'};

public:
    using type = StringConstant<CharT, Chars...>;
    using rep  = std::basic_string_view<CharT>;

    constexpr static rep value {kData, kDataSize};

    constexpr operator rep() const noexcept
    {
        return value;
    }

    constexpr rep operator()() const noexcept
    {
        return value;
    }
};

template<typename T>
struct is_string_constant : public std::false_type
{
    using type = typename std::false_type::type;
};

template<typename CharT, CharT... Chars>
struct is_string_constant<StringConstant<CharT, Chars...>> : public std::true_type
{
    using type = typename std::true_type::type;
};

template<typename T>
using is_string_constant_t = typename is_string_constant<T>::type;

template<typename T>
constexpr inline bool is_string_constant_v = is_string_constant<T>::value;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Commonly used StringConstants               ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using EmptyStringConstant = StringConstant<char>;
using NullStringConstant  = StringConstant<char, '\0'>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// StringConstant metafunction declarations     ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename StringT>
struct filter_out_null_character;

template<typename... StringConstants>
struct concatenate_string_constants;

template<typename String1T, typename String2T>
struct string_compare;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// StringConstant metafunction definitions      ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail
{

template<typename String1T, typename String2T>
struct concatenate_raw_string_constants;

template<typename CharT, CharT... Chars1, CharT... Chars2>
struct concatenate_raw_string_constants<StringConstant<CharT, Chars1...>, StringConstant<CharT, Chars2...>>
{
    // TODO filter null characters out
    using type = StringConstant<CharT, Chars1..., Chars2...>;
};

template<typename String1T, typename String2T>
using concatenate_raw_string_constants_t = typename concatenate_raw_string_constants<String1T, String2T>::type;

template<size_t I, typename String1T, typename String2T>
struct string_compare
{
    using type = typename std::conditional<(String1T::value[I] == String2T::value[I]),
                                           typename string_compare<(I - 1), String1T, String2T>::type, std::false_type>::type;
};

template<typename String1T, typename String2T>
struct string_compare<0, String1T, String2T>
{
    using type = typename std::conditional<(String1T::value[0] == String2T::value[0]), std::true_type, std::false_type>::type;
};

} // namespace detail

template<typename CharT, CharT Head, CharT... Tail>
struct filter_out_null_character<StringConstant<CharT, Head, Tail...>>
{
private:
    using FilteredTail = typename filter_out_null_character<StringConstant<CharT, Tail...>>::type;

public:
    using type = std::conditional_t<Head == '\0', FilteredTail,
                                    detail::concatenate_raw_string_constants_t<StringConstant<CharT, Head>, FilteredTail>>;
};

template<typename CharT>
struct filter_out_null_character<StringConstant<CharT>>
{
    using type = StringConstant<CharT>;
};

template<typename StringT>
using filter_out_null_character_t = typename filter_out_null_character<StringT>::type;

/// @brief Concatenates strings in the order they are provided
/// @tparam First
/// @tparam ...Rest
template<typename StringT, typename... RestT>
struct concatenate_string_constants<StringT, RestT...>
{
    static_assert(is_string_constant_v<StringT>, "shmit::concatenate_string_constants StringT parameter must be "
                                                 "specialization of shmit::StringConstant");

private:
    using ConcatenatedString =
        detail::concatenate_raw_string_constants_t<StringT, typename concatenate_string_constants<RestT...>::type>;

public:
    using type = filter_out_null_character_t<ConcatenatedString>;
};

/// @brief Single string overload
/// @tparam StringConstant
template<typename StringT>
struct concatenate_string_constants<StringT>
{
    static_assert(is_string_constant_v<StringT>, "shmit::concatenate_string_constants StringT parameter must be "
                                                 "specialization of shmit::StringConstant");
    using type = StringT;
};

template<typename... StringConstants>
using concatenate_string_constants_t = typename concatenate_string_constants<StringConstants...>::type;

template<typename String1T, typename String2T>
struct string_compare
{
    static_assert(is_string_constant_v<String1T>, "shmit::concatenate_string_constants String1T parameter must be "
                                                  "specialization of shmit::StringConstant");
    static_assert(is_string_constant_v<String2T>, "shmit::concatenate_string_constants String2T parameter must be "
                                                  "specialization of shmit::StringConstant");

    using type =
        typename std::conditional<(String1T::value.size() == String2T::value.size()),
                                  typename detail::string_compare<(String1T::value.size() - 1), String1T, String2T>::type,
                                  std::false_type>::type;

    constexpr static bool value {type::value};
};

template<typename String1T, typename String2T>
constexpr static bool string_compare_v {string_compare<String1T, String2T>::value};

} // namespace shmit

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// StringConstant global literal operator overload                  ////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename CharT, CharT... Chars>
constexpr static shmit::StringConstant<CharT, Chars...> operator""_ssc()
{
    return {};
}
