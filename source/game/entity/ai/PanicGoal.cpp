#include "game/entity/ai/PanicGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/entity/mob/PathfinderMob.hpp"

PanicGoal::PanicGoal(PathfinderMob* mob, double speedModifier)
    : mMob(mob), mSpeedModifier(speedModifier), mWantedX(0.0), mWantedY(0.0), mWantedZ(0.0)
{
    setRequiredControlFlags(1);
}

PanicGoal::~PanicGoal()
{
}

BlockPos* PanicGoal::lookForWater(Level* level, Entity* entity, int range, int verticalRange)
{
    BlockPos origin(entity);
    BlockPos* result = NULL;
    float bestDistance = static_cast<float>(range * range * verticalRange * 2);

    for (int x = origin.x - range; x <= origin.x + range; ++x)
    {
        for (int y = origin.y - verticalRange; y <= origin.y + verticalRange; ++y)
        {
            for (int z = origin.z - range; z <= origin.z + range; ++z)
            {
                BlockPos pos(x, y, z);
                if (level->isWaterAt(pos))
                {
                    const float distance = static_cast<float>((x - origin.x) * (x - origin.x)
                        + (z - origin.z) * (z - origin.z)
                        + (y - origin.y) * (y - origin.y));
                    if (distance < bestDistance)
                    {
                        result = new BlockPos(pos);
                        bestDistance = distance;
                    }
                }
            }
        }
    }

    return result;
}

bool PanicGoal::findRandomPosition()
{
    boost::shared_ptr<PathfinderMob> mob = boost::dynamic_pointer_cast<PathfinderMob>(mMob->shared_from_this());
    Vec3* position = RandomPos::getPos(mob, 5, 4, -1);
    if (position == NULL)
    {
        return false;
    }

    mWantedX = position->x;
    mWantedY = position->y;
    mWantedZ = position->z;
    return true;
}

bool PanicGoal::canUse()
{
    boost::shared_ptr<LivingEntity> lastHurtByMob = mMob->getLastHurtByMob();
    if (lastHurtByMob.get() == NULL && !mMob->isOnFire())
    {
        return false;
    }

    if (mMob->isOnFire())
    {
        BlockPos* water = lookForWater(mMob->mLevel, mMob, 5, 4);
        if (water != NULL)
        {
            mWantedX = static_cast<double>(water->x);
            mWantedY = static_cast<double>(water->y);
            mWantedZ = static_cast<double>(water->z);
            delete water;
            return true;
        }
    }

    return findRandomPosition();
}

void PanicGoal::start()
{
    mMob->getNavigation()->moveTo(mWantedX, mWantedY, mWantedZ, mSpeedModifier);
}

bool PanicGoal::canContinueToUse()
{
    return !mMob->getNavigation()->isDone();
}

std::wstring PanicGoal::getName()
{
    return std::wstring(L"PanicGoal");
}
