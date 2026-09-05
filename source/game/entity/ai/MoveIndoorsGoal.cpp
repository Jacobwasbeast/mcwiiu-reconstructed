#include "game/entity/ai/MoveIndoorsGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"

class PathfinderMob : public Entity
{
public:
    boost::shared_ptr<Entity> shared_from_this();
    PathNavigation* getNavigation();
    Random* getRandom();
};

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
    boost::shared_ptr<Entity> entity = mMob->shared_from_this();
    BlockPos pos(entity);

    if (mMob->mLevel->isDay())
    {
        if (!mMob->mLevel->isRaining() || mMob->mLevel->canSeeSky(pos))
            return false;
    }

    if (mMob->mLevel->mDimension->isHasSkyLight()
        && mMob->getRandom()->nextInt(50) == 0)
    {
        if (mDoorX != -1
            && mMob->distanceToSqr((double)mDoorX, mMob->mY, (double)mDoorZ) < 4.0)
            return false;

        boost::shared_ptr<Villages> villages = mMob->mLevel->getVillages();
        boost::shared_ptr<Village> village = villages->getClosestVillage(pos, 14);
        if (village)
        {
            boost::shared_ptr<DoorInfo> door = village->getBestDoorInfo(pos);
            if (door)
            {
                mDoorInfo = door;
                return true;
            }
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
    if (!mDoorInfo)
        return;

    BlockPos pos = mDoorInfo->getIndoorPos();
    if (mMob->distanceToSqr(pos) <= 256.0)
    {
        mMob->getNavigation()->moveTo((double)pos.x + 0.5,
                                      (double)pos.y,
                                      (double)pos.z + 0.5,
                                      1.0);
    }
    else
    {
        boost::shared_ptr<PathfinderMob> mob =
            boost::dynamic_pointer_cast<PathfinderMob>(mMob->shared_from_this());
        Vec3* target = Vec3::newTemp((double)pos.x + 0.5,
                                     (double)pos.y,
                                     (double)pos.z + 0.5);
        Vec3* destination = RandomPos::getPosTowards(mob, 14, 3, target);
        if (destination != NULL)
        {
            mMob->getNavigation()->moveTo(destination->x, destination->y,
                                          destination->z, 1.0);
        }
    }
}

void MoveIndoorsGoal::stop()
{
    if (!mDoorInfo)
        return;

    BlockPos pos = mDoorInfo->getIndoorPos();
    mDoorX = pos.x;
    mDoorZ = pos.z;
    mDoorInfo.reset();
}

std::wstring MoveIndoorsGoal::getName()
{
    return std::wstring(L"MoveIndoors");
}
