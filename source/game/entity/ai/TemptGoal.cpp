#include "game/entity/ai/TemptGoal.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

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
    mDelayTemptCounter = 0;
    mRunning = false;
    mItems.data = items.data;
    mItems.length = items.length;
    mItemAuxValues.data = itemAuxValues.data;
    mItemAuxValues.length = itemAuxValues.length;
    mCanScare = canScare;
    mField0x61 = false;
    setRequiredControlFlags(3);
}

TemptGoal::~TemptGoal() = default;

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
    if (!mPlayer.lock())
    {
        return false;
    }

    mMob->setDespawnProtected();
    return shouldFollowItem(mPlayer.lock()->getMainHandItem())
        || shouldFollowItem(mPlayer.lock()->getOffhandItem());
}

bool TemptGoal::canContinueToUse()
{
    if (mCanScare)
    {
        boost::shared_ptr<Player> player = mPlayer.lock();
        if (!player)
        {
            return false;
        }

        if (mMob->distanceToSqr(player) < 36.0)
        {
            if (player->distanceToSqr(mTargetX, mTargetY, mTargetZ) > 0.010000000000000002)
            {
                mDelayTemptCounter = 100;
                return false;
            }

            double xRotDelta = fabs((double)player->mXRot - mTargetXRot);
            double yRotDelta = fabs((double)player->mYRot - mTargetYRot);
            if (xRotDelta > 360.0 - xRotDelta)
            {
                xRotDelta = 360.0 - xRotDelta;
            }
            if (yRotDelta > 360.0 - yRotDelta)
            {
                yRotDelta = 360.0 - yRotDelta;
            }
            if (xRotDelta > 5.0 || yRotDelta > 5.0)
            {
                mDelayTemptCounter = 100;
                return false;
            }
        }
        else
        {
            mTargetX = player->mX;
            mTargetY = player->mY;
            mTargetZ = player->mZ;
        }

        mTargetXRot = (double)player->mXRot;
        mTargetYRot = (double)player->mYRot;
    }

    return canUse();
}

void TemptGoal::start()
{
    boost::shared_ptr<Player> player = mPlayer.lock();
    mTargetX = player->mX;
    mTargetY = player->mY;
    mTargetZ = player->mZ;
    mTargetXRot = (double)player->mXRot;
    mTargetYRot = (double)player->mYRot;
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
    mMob->getLookControl()->setLookAt(mPlayer.lock(), (float)mMob->getMaxHeadYRot() + 20.0f,
                                      (float)mMob->getMaxHeadXRot());

    if (mMob->distanceToSqr(mPlayer.lock()) < 6.25)
    {
        mMob->getNavigation()->stop();
    }
    else
    {
        mMob->getNavigation()->moveTo(mPlayer.lock(), mSpeedModifier);
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
