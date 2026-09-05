#include "game/entity/ai/MoveTowardsRestrictionGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/BlockPos.hpp"

class PathfinderMob : public Entity
{
public:
    bool isWithinRestriction();
    BlockPos getRestrictCenter();
    boost::shared_ptr<Entity> shared_from_this();
    PathNavigation* getNavigation();
};

MoveTowardsRestrictionGoal::MoveTowardsRestrictionGoal(PathfinderMob* mob, double speedModifier)
    : mMob(mob),
      mWantedX(0.0),
      mWantedY(0.0),
      mWantedZ(0.0),
      mSpeedModifier(speedModifier)
{
    setRequiredControlFlags(1);
}

MoveTowardsRestrictionGoal::~MoveTowardsRestrictionGoal()
{
}

bool MoveTowardsRestrictionGoal::canUse()
{
    if (mMob->isWithinRestriction())
    {
        return false;
    }

    BlockPos restrictionCenter = mMob->getRestrictCenter();
    boost::shared_ptr<PathfinderMob> mob =
        boost::dynamic_pointer_cast<PathfinderMob>(mMob->shared_from_this());
    Vec3* target = Vec3::newTemp((double)restrictionCenter.x,
                                  (double)restrictionCenter.y,
                                  (double)restrictionCenter.z);
    Vec3* position = RandomPos::getPosTowards(mob, 16, 7, target);
    if (position == NULL)
    {
        return false;
    }

    mWantedX = position->x;
    mWantedY = position->y;
    mWantedZ = position->z;
    return true;
}

bool MoveTowardsRestrictionGoal::canContinueToUse()
{
    return !mMob->getNavigation()->isDone();
}

void MoveTowardsRestrictionGoal::start()
{
    mMob->getNavigation()->moveTo(mWantedX, mWantedY, mWantedZ, mSpeedModifier);
}

std::wstring MoveTowardsRestrictionGoal::getName()
{
    return std::wstring(L"MoveTowardsRestriction");
}
