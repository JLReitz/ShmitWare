#pragma once

#include "_Detail/Tuple.hpp"

namespace shmit
{
namespace data
{

template<typename TupleT, typename ToPurgeT>
using purge_tuple = _detail::remove_type_from_tuple_recursive<TupleT, ToPurgeT>;

template<typename TupleT, typename ToPurgeT>
using purge_tuple_t = typename purge_tuple<TupleT, ToPurgeT>::type;

} // namespace data
} // namespace shmit