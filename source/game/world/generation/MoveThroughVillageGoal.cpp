#include "game/world/generation/MoveThroughVillageGoal.hpp"

#include <climits>

#include "game/core/Mth.hpp"
#include "game/core/Path.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"
#include "game/world/block/BlockPos.hpp"

class Level
{
public:
    bool isDay();
    boost::shared_ptr<Villages> getVillages();
};

class PathfinderMob : public Entity
{
public:
    PathNavigation* getNavigation();
};

MoveThroughVillageGoal::MoveThroughVillageGoal(PathfinderMob* mob, double speedModifier, bool onlyAtNight)
    : Goal(),
      mMob(mob),
      mSpeedModifier(speedModifier),
      mPath(NULL),
      mDoorInfo(),
      mOnlyAtNight(onlyAtNight),
      mVisited()
{
    setRequiredControlFlags(1);
}

MoveThroughVillageGoal::~MoveThroughVillageGoal()
{
    delete mPath;
}

void MoveThroughVillageGoal::updateVisited()
{
    if (mVisited.size() > 15)
        mVisited.erase(mVisited.begin());
}

bool MoveThroughVillageGoal::hasVisited(boost::shared_ptr<DoorInfo> door)
{
    for (std::vector<boost::weak_ptr<DoorInfo> >::iterator it = mVisited.begin();
         it != mVisited.end();)
    {
        boost::shared_ptr<DoorInfo> visited = it->lock();
        if (!visited)
        {
            it = mVisited.erase(it);
            continue;
        }

        if (visited->getPos().equals(door->getPos()))
            return true;
        ++it;
    }
    return false;
}

boost::shared_ptr<DoorInfo> MoveThroughVillageGoal::getNextDoorInfo(boost::shared_ptr<Village> village)
{
    boost::shared_ptr<DoorInfo> result;
    int closestDistance = INT_MAX;
    BlockPos mobPos(Mth::floor(mMob->mX), Mth::floor(mMob->mY), Mth::floor(mMob->mZ));
    std::vector<boost::shared_ptr<DoorInfo> >* doors = village->getDoorInfos();
    for (std::vector<boost::shared_ptr<DoorInfo> >::iterator it = doors->begin();
         it != doors->end(); ++it)
    {
        boost::shared_ptr<DoorInfo> door = *it;
        int distance = door->distanceToSqr(mobPos.x, mobPos.y, mobPos.z);
        if (distance < closestDistance && !hasVisited(door))
        {
            closestDistance = distance;
            result = door;
        }
    }
    return result;
}

bool MoveThroughVillageGoal::canUse()
{
    updateVisited();
    if (mOnlyAtNight && mMob->mLevel->isDay())
        return false;

    boost::shared_ptr<Villages> villages = mMob->mLevel->getVillages();
    boost::shared_ptr<Entity> entity = mMob->shared_from_this();
    BlockPos mobPos(entity);
    boost::shared_ptr<Village> village = villages->getClosestVillage(mobPos, 32);
    if (!village)
        return false;

    mDoorInfo = getNextDoorInfo(village);
    if (!mDoorInfo)
        return false;

    PathNavigation* navigation = mMob->getNavigation();
    GroundPathNavigation* groundNavigation = static_cast<GroundPathNavigation*>(navigation);
    bool canOpenDoors = groundNavigation->canOpenDoors();
    groundNavigation->setCanOpenDoors(false);
    delete mPath;
    mPath = navigation->createPath(mDoorInfo->getPos());
    groundNavigation->setCanOpenDoors(canOpenDoors);
    if (mPath != NULL)
        return true;

    boost::shared_ptr<PathfinderMob> mob =
        boost::static_pointer_cast<PathfinderMob>(entity);
    BlockPos doorPos = mDoorInfo->getPos();
    Vec3* target = Vec3::newTemp((double)doorPos.x, (double)doorPos.y, (double)doorPos.z);
    Vec3* position = RandomPos::getPosTowards(mob, 10, 7, target);
    if (position == NULL)
        return false;

    groundNavigation->setCanOpenDoors(false);
    delete mPath;
    mPath = navigation->createPath(position->x, position->y, position->z);
    groundNavigation->setCanOpenDoors(canOpenDoors);
    return mPath != NULL;
}

bool MoveThroughVillageGoal::canContinueToUse()
{
    if (mMob->getNavigation()->isDone())
        return false;
    if (!mDoorInfo)
        return false;

    double distance = mMob->distanceToSqr(mDoorInfo->getPos());
    float limit = mMob->mBbWidth + 4.0f;
    return (double)(limit * limit) < distance;
}

void MoveThroughVillageGoal::start()
{
    mMob->getNavigation()->moveTo(mPath, mSpeedModifier);
    mPath = NULL;
}

void MoveThroughVillageGoal::stop()
{
    if (!mDoorInfo)
        return;

    if (mMob->getNavigation()->isDone() ||
        mMob->distanceToSqr(mDoorInfo->getPos()) < 16.0)
        mVisited.push_back(boost::weak_ptr<DoorInfo>(mDoorInfo));
}

std::wstring MoveThroughVillageGoal::getName()
{
    return std::wstring(L"MoveThroughVillage");
}
