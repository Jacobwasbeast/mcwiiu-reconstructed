#include "game/entity/ai/RestrictOpenDoorGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/DoorInfo.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"

class Entity;

class Level
{
public:
    bool isDay();
    boost::shared_ptr<Villages> getVillages();
};

class PathfinderMob
{
public:
    Level* mLevel;
    boost::shared_ptr<Entity> shared_from_this();
    GroundPathNavigation* getNavigation();
};

RestrictOpenDoorGoal::RestrictOpenDoorGoal(PathfinderMob* mob)
    : Goal(),
      mMob(mob),
      mDoor(NULL),
      mDoorInfo()
{
}

RestrictOpenDoorGoal::~RestrictOpenDoorGoal()
{
}

bool RestrictOpenDoorGoal::canUse()
{
    if (mMob->mLevel->isDay())
        return false;

    BlockPos pos(mMob->shared_from_this());
    boost::shared_ptr<Villages> villages = mMob->mLevel->getVillages();
    boost::shared_ptr<Village> village = villages->getClosestVillage(pos, 16);
    if (village == NULL)
        return false;

    boost::shared_ptr<DoorInfo> doorInfo = village->getClosestDoorInfo(pos);
    if (doorInfo == NULL)
        return false;

    mDoor = doorInfo.get();
    mDoorInfo = doorInfo;
    return mDoor->distanceToInsideSqr(pos) <= 2.25;
}

bool RestrictOpenDoorGoal::canContinueToUse()
{
    if (mMob->mLevel->isDay())
        return false;

    if (mDoorInfo == NULL)
        return false;

    if (mDoor->isRemoved())
        return false;

    BlockPos pos(mMob->shared_from_this());
    return mDoor->isInsideSide(pos);
}

void RestrictOpenDoorGoal::start()
{
    mMob->getNavigation()->setCanOpenDoors(false);
    mMob->getNavigation()->setCanPassDoors(false);
}

void RestrictOpenDoorGoal::stop()
{
    mMob->getNavigation()->setCanOpenDoors(true);
    mMob->getNavigation()->setCanPassDoors(true);
    mDoor = NULL;
    mDoorInfo.reset();
}

void RestrictOpenDoorGoal::tick()
{
    if (mDoorInfo != NULL)
        mDoor->incBookingCount();
}

std::wstring RestrictOpenDoorGoal::getName()
{
    return std::wstring(L"RestrictOpenDoor");
}
