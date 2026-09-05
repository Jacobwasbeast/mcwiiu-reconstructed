#include "game/entity/ai/CircleAroundAnchorGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

CircleAroundAnchorGoal::CircleAroundAnchorGoal(Mob* mob, float speedModifier,
                                               Goal::FloatRange distanceRange,
                                               int distanceInterval,
                                               Goal::FloatRange heightRange,
                                               Goal::FloatRange verticalRange,
                                               int verticalInterval,
                                               float minDistance)
    : Goal()
    , mMob(mob)
    , mSpeedModifier(speedModifier)
    , mDistanceMin(distanceRange.min)
    , mDistanceMax(distanceRange.max)
    , mDistanceInterval(distanceInterval)
    , mHeightMin(heightRange.min)
    , mHeightMax(heightRange.max)
    , mVerticalMin(verticalRange.min)
    , mVerticalMax(verticalRange.max)
    , mVerticalInterval(verticalInterval)
    , mMinDistanceSquared(minDistance * minDistance)
    , mAnchor(NULL)
    , mCurrentDistance(0.0f)
    , mCurrentHeight(0.0f)
    , mDirection(1.0f)
    , mAngle(0.0f)
{
    setRequiredControlFlags(3);
}

CircleAroundAnchorGoal::~CircleAroundAnchorGoal()
{
    delete mAnchor;
}

double CircleAroundAnchorGoal::calculateHeightOffset() const
{
    double height = mVerticalMin;
    if (mVerticalMax - height > 0.0f)
    {
        height += mMob->mRandom->nextFloat(mVerticalMax - height);
    }
    return height;
}

void CircleAroundAnchorGoal::setAnchorAboveTarget()
{
    boost::shared_ptr<Entity> target = mMob->getTarget();
    if (!target)
    {
        return;
    }

    Vec3* position = target->getPos(0.0f)->add(0.0, (double)mHeightMin, 0.0);
    if (mHeightMax - mHeightMin > 0.0f)
    {
        position = position->add(0.0, (double)mMob->mRandom->nextFloat(mHeightMax - mHeightMin), 0.0);
    }
    if (position->y <= 0.0)
    {
        position->y = (double)mMob->mRandom->nextInt(256) + 1.0;
    }
    delete mAnchor;
    mAnchor = Vec3::newPermanent(position);
}

void CircleAroundAnchorGoal::selectNext()
{
    if (mAnchor == NULL)
    {
        mAnchor = Vec3::newPermanent(mMob->getPos(0.0f));
        mAnchor->y += 20.0;
        if (mAnchor->y < 256.0)
        {
            mAngle += mDirection * 30.0f * 0.01745329238474369f;
            mAnchor = mAnchor->add((double)(mCurrentDistance * Mth::cos(mAngle)),
                                   (double)mCurrentHeight,
                                   (double)(mCurrentDistance * Mth::sin(mAngle)));
        }
        else
        {
            mAnchor->y = 236.0;
        }
    }
    else
    {
        mAngle += mDirection * 30.0f * 0.01745329238474369f;
    }

    Vec3* wanted = mAnchor->add((double)(mCurrentDistance * Mth::cos(mAngle)),
                                (double)mCurrentHeight,
                                (double)(mCurrentDistance * Mth::sin(mAngle)));
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation != NULL)
    {
        navigation->moveTo(wanted->x, wanted->y, wanted->z, mSpeedModifier);
    }
    else
    {
        mMob->getMoveControl()->setWantedPosition(wanted->x, wanted->y, wanted->z, mSpeedModifier);
    }
}

void CircleAroundAnchorGoal::start()
{
    Random* random = mMob->mRandom;
    mCurrentDistance = random->nextFloat(mDistanceMin, mDistanceMax);
    mCurrentHeight = (float)calculateHeightOffset();
    mDirection = random->nextBoolean() ? 1.0f : -1.0f;
    if (mMob->getTarget())
    {
        setAnchorAboveTarget();
    }
    selectNext();
}

void CircleAroundAnchorGoal::stop()
{
    if (mMob->getNavigation() != NULL)
    {
        mMob->getNavigation()->stop();
    }
    else
    {
        mMob->getMoveControl()->wait();
    }
}

bool CircleAroundAnchorGoal::canUse()
{
    return true;
}

bool CircleAroundAnchorGoal::touchingTarget() const
{
    if (mMob->getNavigation() != NULL)
    {
        return mMob->getNavigation()->isDone();
    }

    MoveControl* moveControl = mMob->getMoveControl();
    Vec3* wanted = Vec3::newTemp(moveControl->getWantedX(), moveControl->getWantedY(),
                                 moveControl->getWantedZ());
    return mMob->getPos(0.0f)->distanceToSqr(wanted) < (double)mMinDistanceSquared;
}

void CircleAroundAnchorGoal::tick()
{
    Random* random = mMob->mRandom;
    if (mMob->getTarget())
    {
        setAnchorAboveTarget();
    }

    if (mVerticalMin < mVerticalMax
        && (mVerticalInterval == 0 || random->nextInt(mVerticalInterval) == 0))
    {
        mCurrentHeight = (float)calculateHeightOffset();
    }

    if (mDistanceMax <= mDistanceMin
        || (mDistanceInterval != 0 && random->nextInt(mDistanceInterval) != 0))
    {
        selectNext();
    }
    else
    {
        mCurrentDistance += 1.0f;
        selectNext();
    }

    if (touchingTarget())
    {
        selectNext();
    }
}

std::wstring CircleAroundAnchorGoal::getName()
{
    return std::wstring(L"CircleAroundAnchorGoal");
}
