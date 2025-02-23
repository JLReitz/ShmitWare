#pragma once

#include <chrono>

namespace shmit
{
namespace time
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Namespace metafunction declarations             ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename Duration>
struct no_timeout;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Namespace metafunction definitions in alphabetical order ////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename Duration>
struct no_timeout
{
    constexpr static Duration value {Duration::zero()};
};

template<typename Duration>
constexpr static Duration no_timeout_v = no_timeout<Duration>::value;

} // namespace time
} // namespace shmit