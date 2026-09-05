#include "game/entity/ai/PlayGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Villager.hpp"

PlayGoal::PlayGoal(Villager* villager, double speedModifier)
    : mVillager(villager), mTarget(), mSpeedModifier(speedModifier),
      mPlayPosition(1.0, 1.0, 1.0), mPlayTime(0)
{
    setRequiredControlFlags(1);
}

PlayGoal::~PlayGoal()
{
}

bool PlayGoal::canUse()
{
    if (mVillager->getAge() >= 0)
        return false;

    if (mVillager->getRandom()->nextInt(400) != 0)
        return false;

    AABB* searchBox = mVillager->mBoundingBox->grow(6.0, 3.0, 6.0);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mVillager->mLevel->getEntitiesOfClass(typeid(Villager), searchBox);
    double closestDistance = 1.7976931348623157e+308;
    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Villager> candidate =
            boost::dynamic_pointer_cast<Villager>(*it);
        if (!candidate || candidate.get() == mVillager || candidate->isChasing()
            || candidate->getAge() >= 0)
            continue;

        double distance = mVillager->distanceToSqr(*it);
        if (distance <= closestDistance)
        {
            closestDistance = distance;
            mTarget = candidate;
        }
    }

    delete entities;
    if (mTarget)
        return true;

    boost::shared_ptr<Entity> self = mVillager->shared_from_this();
    boost::shared_ptr<PathfinderMob> mob =
        boost::dynamic_pointer_cast<PathfinderMob>(self);
    return RandomPos::getPos(mob, 16, 3, -1) != 0;
}

bool PlayGoal::canContinueToUse()
{
    return mPlayTime > 0 && mTarget;
}

void PlayGoal::start()
{
    if (mTarget)
        mTarget->setChasing(true);
    mPlayTime = 1000;
}

void PlayGoal::stop()
{
    if (mTarget)
        mTarget->setChasing(false);
    mTarget.reset();
}

void PlayGoal::tick()
{
    --mPlayTime;
    if (mTarget)
    {
        if (mVillager->distanceToSqr(mTarget) > 4.0)
            mVillager->getNavigation()->moveTo(mTarget, mSpeedModifier);
        return;
    }

    PathNavigation* navigation = mVillager->getNavigation();
    if (navigation->isDone())
    {
        boost::shared_ptr<Entity> self = mVillager->shared_from_this();
        boost::shared_ptr<PathfinderMob> mob =
            boost::dynamic_pointer_cast<PathfinderMob>(self);
        Vec3* position = RandomPos::getPos(mob, 16, 3, -1);
        if (position != 0)
            navigation->moveTo(position->x, position->y, position->z, mSpeedModifier);
    }
}

std::wstring PlayGoal::getName()
{
    return std::wstring(L"Play");
}
