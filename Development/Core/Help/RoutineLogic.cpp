#include <Core/Help/RoutineLogic.hpp>

namespace shmit
{
namespace help
{

void WaitForPassCondition(RepeatableCheck condition)
{
    bool condition_passed {false};
    do
    {
        condition_passed = condition();
    } while (!condition_passed);
}

void BlockOnPassCondition(RepeatableCheck condition)
{
    // TODO call to system block method between checks
    // For now pass to WaitForPassCondition
    WaitForPassCondition(condition);
}

} // namespace help
} // namespace shmit
