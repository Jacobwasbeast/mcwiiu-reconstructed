#include "game/entity/ai/OwnerHurtTargetGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/TamableAnimal.hpp"

OwnerHurtTargetGoal::OwnerHurtTargetGoal(TamableAnimal* tamable)
    : TargetGoal(tamable, false)
    , mTamable(tamable)
    , mOwnerLastHurt()
    , mTimestamp(0)
{
    setRequiredControlFlags(1);
}

OwnerHurtTargetGoal::~OwnerHurtTargetGoal()
{
}

bool OwnerHurtTargetGoal::canUse()
{
    if (!mTamable->isTame())
        return false;

    boost::shared_ptr<Entity> owner = mTamable->getOwner();
    boost::shared_ptr<LivingEntity> livingOwner = boost::dynamic_pointer_cast<LivingEntity>(owner);
    if (!livingOwner)
        return false;

    boost::shared_ptr<LivingEntity> ownerLastHurt = livingOwner->getLastHurtMob();
    mOwnerLastHurt = ownerLastHurt;
    int timestamp = livingOwner->getLastHurtMobTimestamp();
    if (timestamp == mTimestamp)
        return false;

    if (!canAttack(mOwnerLastHurt, false))
        return false;

    return mTamable->wantsToAttack(mOwnerLastHurt, livingOwner);
}

void OwnerHurtTargetGoal::start()
{
    mMob->setTarget(mOwnerLastHurt);

    boost::shared_ptr<Entity> owner = mTamable->getOwner();
    boost::shared_ptr<LivingEntity> livingOwner = boost::dynamic_pointer_cast<LivingEntity>(owner);
    if (livingOwner != NULL)
    {
        mTimestamp = livingOwner->getLastHurtMobTimestamp();
        TargetGoal::start();
    }
}

std::wstring OwnerHurtTargetGoal::getName()
{
    return std::wstring(L"OwnerHurtTargetGoal");
}
