#include "game/entity/ai/OwnerHurtByTargetGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/TamableAnimal.hpp"

OwnerHurtByTargetGoal::OwnerHurtByTargetGoal(TamableAnimal* tamableAnimal)
    : TargetGoal(tamableAnimal, false),
      mTamableAnimal(tamableAnimal),
      mOwnerLastHurtBy(),
      mTimestamp(0)
{
    setRequiredControlFlags(1);
}

bool OwnerHurtByTargetGoal::canUse()
{
    if (!mTamableAnimal->isTame())
        return false;

    boost::shared_ptr<LivingEntity> owner = boost::dynamic_pointer_cast<LivingEntity>(mTamableAnimal->getOwner());
    if (!owner)
        return false;

    boost::shared_ptr<LivingEntity> target = owner->getLastHurtByMob();
    mOwnerLastHurtBy = target;

    if (owner->getLastHurtByMobTimestamp() == mTimestamp)
        return false;

    if (!canAttack(mOwnerLastHurtBy.lock(), false))
        return false;

    if (!mTamableAnimal->wantsToAttack(mOwnerLastHurtBy.lock(), owner))
        return false;

    if (mMob->mHurtByPlayerOnPeaceful)
        return false;

    return true;
}

void OwnerHurtByTargetGoal::start()
{
    mMob->setTarget(mOwnerLastHurtBy.lock());

    boost::shared_ptr<LivingEntity> owner = boost::dynamic_pointer_cast<LivingEntity>(mTamableAnimal->getOwner());
    if (owner)
        mTimestamp = owner->getLastHurtByMobTimestamp();

    TargetGoal::start();
}

std::wstring OwnerHurtByTargetGoal::getName()
{
    return std::wstring(L"OwnersHurtByTarget");
}
