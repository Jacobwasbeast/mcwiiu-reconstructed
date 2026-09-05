#include "game/entity/ai/InteractGoal.hpp"

#include <string>

InteractGoal::InteractGoal(Mob* mob, const std::type_info& lookAtType, float lookDistance,
                           float probability)
    : LookAtPlayerGoal(mob, lookAtType, lookDistance, probability)
{
    setRequiredControlFlags(3);
}

InteractGoal::~InteractGoal()
{
}

std::wstring InteractGoal::getName()
{
    return std::wstring(L"Interact");
}
