#include "game/entity/ai/TemptGoal.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/item/ItemInstance.hpp"

class Level
{
public:
    boost::shared_ptr<LivingEntity> getNearestPlayer(boost::shared_ptr<Entity> entity,
                                                     double distance, double maxYDistance);
};

TemptGoal::TemptGoal(PathfinderMob* mob, double speedModifier,
                     arrayWithLength<Item*> items, bool canScare,
                     arrayWithLength<int> itemAuxValues)
{
    mMob = mob;
    mSpeedModifier = speedModifier;
    mTargetX = 0.0;
    mTargetY = 0.0;
    mTargetZ = 0.0;
    mTargetXRot = 0.0;
    mTargetYRot = 0.0;
    mPlayer.reset();
    mItems.data = items.data;
    mItems.length = items.length;
    mItemAuxValues.data = itemAuxValues.data;
    mItemAuxValues.length = itemAuxValues.length;
    mCanScare = canScare;
    mRunning = false;
    mDelayTemptCounter = 0;
    setRequiredControlFlags(3);
}

TemptGoal::~TemptGoal()
{
}

bool TemptGoal::shouldFollowItem(not_null_ptr<ItemInstance> item)
{
    for (unsigned int i = 0; i < mItems.length; ++i)
    {
        if (item->getItem() == mItems.data[i]
            && (mItemAuxValues.length == 0 || item->getAuxValue() == mItemAuxValues.data[i]))
        {
            return true;
        }
    }

    return false;
}

bool TemptGoal::canUse()
{
    if (mDelayTemptCounter > 0)
    {
        --mDelayTemptCounter;
        return false;
    }

    mPlayer = mMob->mLevel->getNearestPlayer(mMob->shared_from_this(), 10.0, -1.0);
    if (!mPlayer)
    {
        return false;
    }

    return shouldFollowItem(mPlayer->getMainHandItem())
        || shouldFollowItem(mPlayer->getOffhandItem());
}

bool TemptGoal::canContinueToUse()
{
    if (!mCanScare || !mPlayer)
    {
        return false;
    }

    if (mMob->distanceToSqr(mPlayer) < 36.0
        && mMob->distanceToSqr(mTargetX, mTargetY, mTargetZ) > 0.01)
    {
        mDelayTemptCounter = 100;
        return false;
    }

    mTargetX = mPlayer->mX;
    mTargetY = mPlayer->mY;
    mTargetZ = mPlayer->mZ;
    mTargetXRot = (double)mPlayer->mXRot;
    mTargetYRot = (double)mPlayer->mYRot;
    return true;
}

void TemptGoal::start()
{
    mTargetX = mPlayer->mX;
    mTargetY = mPlayer->mY;
    mTargetZ = mPlayer->mZ;
    mTargetXRot = (double)mPlayer->mXRot;
    mTargetYRot = (double)mPlayer->mYRot;
    mRunning = true;
}

void TemptGoal::stop()
{
    mPlayer.reset();
    mMob->getNavigation()->stop();
    mRunning = false;
}

void TemptGoal::tick()
{
    mMob->getLookControl()->setLookAt(mPlayer, mMob->getMaxHeadYRot(),
                                      mMob->getMaxHeadXRot());

    if (mMob->distanceToSqr(mPlayer) < 6.25)
    {
        mMob->getNavigation()->stop();
    }
    else
    {
        mMob->getNavigation()->moveTo(mPlayer, mSpeedModifier);
    }
}

bool TemptGoal::isRunning()
{
    return mRunning;
}

std::wstring TemptGoal::getName()
{
    return std::wstring(L"Tempt");
}
