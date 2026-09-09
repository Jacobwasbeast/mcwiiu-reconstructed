#include "game/entity/ai/FloatGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/JumpControl.hpp"
#include "game/entity/ai/FlyingPathNavigation.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"

FloatGoal::FloatGoal(Mob* mob)
    : Goal(),
      mMob(mob)
{
    setRequiredControlFlags(4);

    PathNavigation* navigation = mob->getNavigation();
    if (GroundPathNavigation* groundNavigation = dynamic_cast<GroundPathNavigation*>(navigation))
    {
        groundNavigation->setCanFloat(true);
    }
    else if (FlyingPathNavigation* flyingNavigation = dynamic_cast<FlyingPathNavigation*>(navigation))
    {
        flyingNavigation->setCanFloat(true);
    }
}

FloatGoal::~FloatGoal()
{
}

bool FloatGoal::canUse()
{
    return mMob->isInWater() || mMob->isInLava();
}

void FloatGoal::tick()
{
    mMob->getNavigation()->tick();
    if (mMob->getRandom()->nextFloat() < 0.8f)
    {
        static_cast<JumpControl*>(mMob->getJumpControl())->jump();
    }
}

std::wstring FloatGoal::getName()
{
    return std::wstring(L"Float");
}
