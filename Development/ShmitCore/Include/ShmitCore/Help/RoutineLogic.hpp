#pragma once

#include "ShmitCore/Platform/Clock.hpp"

#include <functional>

namespace shmit
{
namespace help
{

using RepeatableCheck = std::function<bool(void)>;

void WaitForPassCondition(RepeatableCheck condition);

void BlockOnPassCondition(RepeatableCheck condition);

} // namespace help
} // namespace shmit