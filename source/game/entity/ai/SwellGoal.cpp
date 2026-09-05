#include "game/entity/ai/SwellGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Creeper.hpp"
#include "game/core/Sensing.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/LivingEntity.hpp"

SwellGoal::SwellGoal(Creeper* creeper)
    : mCreeper(creeper)
{
    setRequiredControlFlags(1);
}

SwellGoal::~SwellGoal()
{
}

bool SwellGoal::canUse()
{
    if (mCreeper->getSwellDir() >= 0)
        return false;

    boost::shared_ptr<LivingEntity> target = mCreeper->getTarget();
    return target != NULL && mCreeper->distanceToSqr(target) < 9.0;
}

void SwellGoal::start()
{
    mCreeper->getNavigation()->stop();
    mTarget = mCreeper->getTarget();
}

void SwellGoal::stop()
{
    mTarget.reset();
}

void SwellGoal::tick()
{
    if (mTarget && mCreeper->distanceToSqr(mTarget) <= 49.0
        && mCreeper->getSensing()->canSee(mTarget))
        mCreeper->setSwellDir(1);
    else
        mCreeper->setSwellDir(-1);
}

std::wstring SwellGoal::getName()
{
    return std::wstring(L"Swell");
}
