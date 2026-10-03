#include "game/entity/ai/CircleAroundAnchorGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/Level.hpp"
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
    float height = mVerticalMin;
    if (mVerticalMax - height > 0.0f)
    {
        height += mMob->mLevel->mRandom->nextFloat(mVerticalMax - height);
    }
    return height;
}

void CircleAroundAnchorGoal::setAnchorAboveTarget()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
    {
        return;
    }

    Vec3* position = target->getPos(0.0f)->add(0.0, (double)mHeightMin, 0.0);
    if (mHeightMax - mHeightMin > 0.0f)
    {
        position = position->add(0.0,
                                 (double)mMob->mLevel->mRandom->nextFloat(mHeightMax - mHeightMin),
                                 0.0);
    }
    int seaLevel = mMob->mLevel->getSeaLevel();
    if (position->y <= (double)seaLevel)
    {
        position->y = (double)(seaLevel + 1);
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
        if (mAnchor->y >= 256.0)
        {
            mAnchor->y = 236.0;
        }
    }

    mAngle += mDirection * 30.0f * 0.017453292f;
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
    Random* random = mMob->mLevel->mRandom;
    mCurrentDistance = random->nextFloat() * (mDistanceMax - mDistanceMin) + mDistanceMin;
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
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation != NULL)
    {
        navigation->stop();
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
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation != NULL)
    {
        return navigation->isDone();
    }

    MoveControl* moveControl = mMob->getMoveControl();
    Vec3* position = mMob->getPos(0.0f);
    Vec3* wanted = Vec3::newTemp(moveControl->getWantedX(), moveControl->getWantedY(),
                                 moveControl->getWantedZ());
    return wanted->distanceToSqr(position) < (double)mMinDistanceSquared;
}

void CircleAroundAnchorGoal::tick()
{
    Random* random = mMob->mLevel->mRandom;
    if (mMob->getTarget())
    {
        setAnchorAboveTarget();
    }

    if (mVerticalMin < mVerticalMax
        && (mVerticalInterval == 0 || random->nextInt(mVerticalInterval) == 0))
    {
        mCurrentHeight = (float)calculateHeightOffset();
    }

    if (mDistanceMin < mDistanceMax
        && (mDistanceInterval == 0 || random->nextInt(mDistanceInterval) == 0))
    {
        mCurrentDistance += 1.0f;
        if (mCurrentDistance > mDistanceMax)
        {
            mCurrentDistance = mDistanceMin;
            mDirection = -mDirection;
        }
    }

    Vec3* position = mMob->getPos(0.0f);
    phys::Vec2* current = phys::Vec2::newTemp((float)position->x, (float)position->z);
    phys::Vec2* wanted = phys::Vec2::newTemp((float)mMob->getMoveControl()->getWantedX(),
                                             (float)mMob->getMoveControl()->getWantedZ());
    double distanceToWanted = wanted->distanceToSqr(current);

    if (distanceToWanted < (double)mMinDistanceSquared
        && mMob->getMoveControl()->getWantedY() < mMob->getPos(0.0f)->y
        && !mMob->mLevel->isEmptyBlock(BlockPos(mMob->getPos(0.0f)).below(1)))
    {
        mCurrentHeight = mCurrentHeight > 1.0f ? mCurrentHeight : 1.0f;
        selectNext();
    }

    if (distanceToWanted < (double)mMinDistanceSquared
        && mMob->getMoveControl()->getWantedY() > mMob->getPos(0.0f)->y
        && !mMob->mLevel->isEmptyBlock(BlockPos(mMob->getPos(0.0f)).above(1)))
    {
        mCurrentHeight = mCurrentHeight > -1.0f ? -1.0f : mCurrentHeight;
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
