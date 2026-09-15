#include "game/entity/ai/RunAroundLikeCrazyGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/Player.hpp"

RunAroundLikeCrazyGoal::RunAroundLikeCrazyGoal(AbstractHorse* horse, double speedModifier)
    : mHorse(horse), mSpeedModifier(speedModifier), mWantedX(0.0), mWantedY(0.0), mWantedZ(0.0)
{
    setRequiredControlFlags(1);
}

RunAroundLikeCrazyGoal::~RunAroundLikeCrazyGoal()
{
}

bool RunAroundLikeCrazyGoal::canUse()
{
    if (mHorse->isTamed() || !mHorse->isVehicle())
    {
        return false;
    }

    boost::shared_ptr<PathfinderMob> horse = boost::static_pointer_cast<PathfinderMob>(mHorse->shared_from_this());
    Vec3* target = RandomPos::getPos(horse, 5, 4, -1);
    if (target == NULL)
    {
        return false;
    }

    mWantedX = target->x;
    mWantedY = target->y;
    mWantedZ = target->z;
    return true;
}

void RunAroundLikeCrazyGoal::start()
{
    mHorse->getNavigation()->moveTo(mWantedX, mWantedY, mWantedZ, mSpeedModifier);
}

bool RunAroundLikeCrazyGoal::canContinueToUse()
{
    return !mHorse->isTamed() && !mHorse->getNavigation()->isDone() && mHorse->isVehicle();
}

void RunAroundLikeCrazyGoal::tick()
{
    if (mHorse->isTamed() || mHorse->getRandom()->nextInt(50) != 0)
    {
        return;
    }

    boost::shared_ptr<Entity> passenger = mHorse->getControllingPassenger();
    if (!passenger)
    {
        return;
    }

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(passenger);
    int temper = mHorse->getTemper();
    int maxTemper = mHorse->getMaxTemper();
    if (maxTemper > 0 && mHorse->getRandom()->nextInt(maxTemper) < temper)
    {
        mHorse->tameWithName(player);
        return;
    }

    mHorse->modifyTemper(5);
    mHorse->ejectPassengers();
    mHorse->makeMad();
}

std::wstring RunAroundLikeCrazyGoal::getName()
{
    return std::wstring(L"RunAroundLikeCrazy");
}
