#include "game/entity/ai/NonTameRandomTargetGoal.hpp"

#include "game/entity/mob/TamableAnimal.hpp"

NonTameRandomTargetGoal::NonTameRandomTargetGoal(
    TamableAnimal* tamableAnimal,
    const std::type_info& targetType,
    bool mustSee,
    const Predicate<boost::shared_ptr<Entity> >* selector,
    bool includeRemoved)
    : NearestAttackableTargetGoal(tamableAnimal, targetType, 10, mustSee, false,
                                  selector, true, includeRemoved),
      mTamableAnimal(tamableAnimal)
{
}

NonTameRandomTargetGoal::~NonTameRandomTargetGoal()
{
}

bool NonTameRandomTargetGoal::canUse()
{
    return !mTamableAnimal->isTame() && NearestAttackableTargetGoal::canUse();
}

bool NonTameRandomTargetGoal::canContinueToUse()
{
    return !mTamableAnimal->isTame() && NearestAttackableTargetGoal::canContinueToUse();
}

std::wstring NonTameRandomTargetGoal::getName()
{
    return std::wstring(L"NonTameRandomTarget");
}
