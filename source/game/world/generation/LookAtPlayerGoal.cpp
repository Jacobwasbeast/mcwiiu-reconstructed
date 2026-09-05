#include "game/world/generation/LookAtPlayerGoal.hpp"

#include "game/entity/ai/EntitySelector.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

LookAtPlayerGoal::LookAtPlayerGoal(Mob* mob, const std::type_info& lookAtType, float lookDistance)
    : mMob(mob), mLookAt(), mLookDistance(lookDistance), mLookTime(0), mProbability(0.02f),
      mLookAtType(&lookAtType)
{
    setRequiredControlFlags(2);
}

LookAtPlayerGoal::LookAtPlayerGoal(Mob* mob, const std::type_info& lookAtType, float lookDistance,
                                   float probability)
    : mMob(mob), mLookAt(), mLookDistance(lookDistance), mLookTime(0), mProbability(probability),
      mLookAtType(&lookAtType)
{
    setRequiredControlFlags(2);
}

LookAtPlayerGoal::~LookAtPlayerGoal()
{
}

bool LookAtPlayerGoal::canUse()
{
    if (mMob->getRandom()->nextFloat() >= mProbability)
        return false;

    mLookAt = mMob->mLevel->getClosestEntityOfClass(
        *mLookAtType, mMob->mBoundingBox->grow(mLookDistance, mLookDistance, mLookDistance),
        mMob->shared_from_this());

    return mLookAt.get() != NULL;
}

bool LookAtPlayerGoal::canContinueToUse()
{
    if (mLookAt.get() == NULL)
        return false;
    if (!mLookAt->isAlive())
        return false;
    if (mMob->distanceToSqr(mLookAt) > static_cast<double>(mLookDistance * mLookDistance))
        return false;
    return mLookTime > 0;
}

void LookAtPlayerGoal::start()
{
    mLookTime = 40 + mMob->getRandom()->nextInt(40);
}

void LookAtPlayerGoal::stop()
{
    mLookAt.reset();
    mLookTime = 0;
}

void LookAtPlayerGoal::tick()
{
    --mLookTime;
    if (mLookAt.get() != NULL)
    {
        static_cast<LookControl*>(mMob->getLookControl())->setLookAt(
            mLookAt->mX, mLookAt->mY + mLookAt->getEyeHeight(), mLookAt->mZ,
            static_cast<float>(mMob->getMaxHeadYRot()), static_cast<float>(mMob->getMaxHeadXRot()));
    }
}

std::wstring LookAtPlayerGoal::getName()
{
    return std::wstring(L"LookAtPlayer");
}

bool LookAtPlayerGoal::LookAtPlayerGoalEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    if (!EntitySelector::NO_SPECTATORS->apply(entity))
        return false;
    return entity->isAlive();
}

LookAtPlayerGoal::LookAtPlayerGoalEntitySelector::~LookAtPlayerGoalEntitySelector()
{
}
