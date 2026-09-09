#include "game/entity/ai/MoveIndoorsGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"

MoveIndoorsGoal::MoveIndoorsGoal(PathfinderMob* mob)
    : Goal(),
      mMob(mob),
      mDoorInfo(),
      mDoorX(-1),
      mDoorZ(-1)
{
    setRequiredControlFlags(1);
}

MoveIndoorsGoal::~MoveIndoorsGoal()
{
}

bool MoveIndoorsGoal::canUse()
{
    BlockPos pos(mMob->shared_from_this());

    if (mMob->mLevel->isDay())
    {
        if (!mMob->mLevel->isRaining())
            return false;

        if (mMob->mLevel->getBiome(pos)->hasRain())
            return false;
    }

    if (mMob->mLevel->mDimension->isHasSkyLight()
        && mMob->getRandom()->nextInt(50) == 0)
    {
        if (mDoorX != -1
            && mMob->distanceToSqr((double)mDoorX, mMob->mY, (double)mDoorZ) < 4.0)
            return false;

        boost::shared_ptr<Village> village =
            mMob->mLevel->getVillages()->getClosestVillage(pos, 14);
        if (village)
        {
            boost::shared_ptr<DoorInfo> doorInfo = village->getBestDoorInfo(pos);
            mDoorInfo = doorInfo;
            return doorInfo.get() != NULL;
        }
    }

    return false;
}

bool MoveIndoorsGoal::canContinueToUse()
{
    return !mMob->getNavigation()->isDone();
}

void MoveIndoorsGoal::start()
{
    mDoorX = -1;

    boost::shared_ptr<DoorInfo> doorInfo = mDoorInfo.lock();
    if (!doorInfo)
    {
        mDoorInfo.reset();
        return;
    }

    BlockPos pos = doorInfo->getIndoorPos();
    if (mMob->distanceToSqr(pos) <= 256.0)
    {
        mMob->getNavigation()->moveTo((double)pos.x + 0.5,
                                      (double)pos.y,
                                      (double)pos.z + 0.5,
                                      1.0);
    }
    else
    {
        Vec3* destination = RandomPos::getPosTowards(
            boost::dynamic_pointer_cast<PathfinderMob>(mMob->shared_from_this()),
            14,
            3,
            Vec3::newTemp((double)pos.x + 0.5, (double)pos.y, (double)pos.z + 0.5));
        if (destination != NULL)
        {
            mMob->getNavigation()->moveTo(destination->x, destination->y,
                                          destination->z, 1.0);
        }
    }
}

void MoveIndoorsGoal::stop()
{
    boost::shared_ptr<DoorInfo> doorInfo = mDoorInfo.lock();
    if (!doorInfo)
    {
        mDoorInfo.reset();
        return;
    }

    mDoorX = doorInfo->getIndoorPos().x;
    mDoorZ = doorInfo->getIndoorPos().z;
    mDoorInfo.reset();
}

std::wstring MoveIndoorsGoal::getName()
{
    return std::wstring(L"MoveIndoors");
}
