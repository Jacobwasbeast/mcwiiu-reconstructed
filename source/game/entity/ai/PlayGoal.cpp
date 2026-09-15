#include "game/entity/ai/PlayGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Villager.hpp"

PlayGoal::PlayGoal(Villager* villager, double speedModifier)
    : mVillager(villager), mTarget(), mSpeedModifier(speedModifier),
      mPlayPosition(0.0, 0.0, 0.0), mPlayTime(0)
{
    mTarget.reset();
    setRequiredControlFlags(1);
}

PlayGoal::~PlayGoal()
{
    mTarget.reset();
    mVillager = 0;
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
        boost::shared_ptr<Entity> entity = *it;
        if (entity.get() == mVillager)
            continue;

        boost::shared_ptr<Villager> candidate =
            boost::dynamic_pointer_cast<Villager>(entity);
        if (candidate->isChasing() || candidate->getAge() >= 0)
            continue;

        double distance = candidate->distanceToSqr(mVillager->shared_from_this());
        if (distance <= closestDistance)
        {
            closestDistance = distance;
            mTarget = candidate;
        }
    }

    delete entities;

    if (!mTarget.lock())
    {
        boost::shared_ptr<PathfinderMob> mob =
            boost::dynamic_pointer_cast<PathfinderMob>(mVillager->shared_from_this());
        if (RandomPos::getPos(mob, 16, 3, -1) == 0)
            return false;
    }
    return true;
}

bool PlayGoal::canContinueToUse()
{
    return mPlayTime > 0 && mTarget.lock();
}

void PlayGoal::start()
{
    if (mTarget.lock())
        mVillager->setChasing(true);
    mPlayTime = 1000;
}

void PlayGoal::stop()
{
    mVillager->setChasing(false);
    mTarget.reset();
}

void PlayGoal::tick()
{
    --mPlayTime;

    if (mTarget.lock())
    {
        if (mVillager->distanceToSqr(mTarget.lock()) > 4.0)
            mVillager->getNavigation()->moveTo(mTarget.lock(), mSpeedModifier);
        return;
    }

    if (mVillager->getNavigation()->isDone())
    {
        boost::shared_ptr<PathfinderMob> mob =
            boost::dynamic_pointer_cast<PathfinderMob>(mVillager->shared_from_this());
        Vec3* position = RandomPos::getPos(mob, 16, 3, -1);
        if (position != 0)
        {
            mVillager->getNavigation()->moveTo(position->x, position->y, position->z,
                                               mSpeedModifier);
        }
    }
}

std::wstring PlayGoal::getName()
{
    return L"Play";
}
