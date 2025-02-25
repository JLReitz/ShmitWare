#pragma once

#include <tuple>

namespace shmit
{
namespace data
{
namespace _detail
{

template<typename TupleT, typename ToRemoveT>
struct remove_type_from_tuple_recursive;

// Base case with empty tuple
template<typename ToRemoveT>
struct remove_type_from_tuple_recursive<std::tuple<>, ToRemoveT>
{
    using type = std::tuple<>;
};

// Case where first element in tuple is of type to be removed
template<typename ToRemoveT, typename... RestT>
struct remove_type_from_tuple_recursive<std::tuple<ToRemoveT, RestT...>, ToRemoveT>
{
    using type = typename remove_type_from_tuple_recursive<std::tuple<RestT...>, ToRemoveT>::type;
};

// Case where the first element in tupe is not of type to be removed
template<typename ToRemoveT, typename FirstT, typename... RestT>
struct remove_type_from_tuple_recursive<std::tuple<FirstT, RestT...>, ToRemoveT>
{
    using type =
        decltype(std::tuple_cat(std::tuple<FirstT> {},
                                typename remove_type_from_tuple_recursive<std::tuple<RestT...>, ToRemoveT>::type {}));
};

} // namespace _detail
} // namespace data
} // namespace shmit