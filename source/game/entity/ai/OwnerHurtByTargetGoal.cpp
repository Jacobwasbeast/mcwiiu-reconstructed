#include "game/entity/ai/OwnerHurtByTargetGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/TamableAnimal.hpp"

OwnerHurtByTargetGoal::OwnerHurtByTargetGoal(TamableAnimal* tamableAnimal)
    : TargetGoal(tamableAnimal, false),
      mTamableAnimal(tamableAnimal),
      mOwnerLastHurtByTimestamp(0),
      mOwnerLastHurtBy(),
      mTimestamp(0)
{
    setRequiredControlFlags(1);
}

OwnerHurtByTargetGoal::~OwnerHurtByTargetGoal()
{
}

bool OwnerHurtByTargetGoal::canUse()
{
    boost::shared_ptr<LivingEntity> owner = mTamableAnimal->getOwner();
    if (!owner)
        return false;

    boost::shared_ptr<LivingEntity> target = owner->getLastHurtByMob();
    if (!target)
        return false;

    mOwnerLastHurtBy = target;
    mOwnerLastHurtByTimestamp = owner->getLastHurtByMobTimestamp();

    if (mOwnerLastHurtByTimestamp == mTimestamp)
        return false;

    if (!canAttack(target, false))
        return false;

    if (!mTamableAnimal->wantsToAttack(target, owner))
        return false;

    if (mMob->mHurtByPlayerOnPeaceful)
        return false;

    return true;
}

void OwnerHurtByTargetGoal::start()
{
    mMob->setTarget(mOwnerLastHurtBy);

    boost::shared_ptr<LivingEntity> owner = mTamableAnimal->getOwner();
    if (owner)
        mTimestamp = owner->getLastHurtByMobTimestamp();

    TargetGoal::start();
}

std::wstring OwnerHurtByTargetGoal::getName()
{
    return std::wstring(L"OwnersHurtByTarget");
}
