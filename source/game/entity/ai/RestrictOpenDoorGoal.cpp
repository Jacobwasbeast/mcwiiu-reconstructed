#include "game/entity/ai/RestrictOpenDoorGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/DoorInfo.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"

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
    GroundPathNavigation* navigation = static_cast<GroundPathNavigation*>(mMob->getNavigation());
    navigation->setCanOpenDoors(false);
    navigation->setCanPassDoors(false);
}

void RestrictOpenDoorGoal::stop()
{
    GroundPathNavigation* navigation = static_cast<GroundPathNavigation*>(mMob->getNavigation());
    navigation->setCanOpenDoors(true);
    navigation->setCanPassDoors(true);
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
