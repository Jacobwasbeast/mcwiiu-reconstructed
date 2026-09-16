#include "game/world/generation/DefendVillageTargetGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/core/Creeper.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/VillagerGolem.hpp"

DefendVillageTargetGoal::DefendVillageTargetGoal(VillagerGolem* golem)
    : TargetGoal(golem, false, true)
{
    mGolem = golem;
    setRequiredControlFlags(1);
}

DefendVillageTargetGoal::~DefendVillageTargetGoal()
{
}

bool DefendVillageTargetGoal::canUse()
{
    boost::shared_ptr<Village> village = mGolem->getVillage();
    if (!village)
    {
        return false;
    }

    mVillageAggressorTarget = village->getClosestAggressor(
        boost::static_pointer_cast<LivingEntity>(mGolem->shared_from_this()));

    boost::shared_ptr<LivingEntity> aggressor = mVillageAggressorTarget.lock();
    if (aggressor && boost::dynamic_pointer_cast<Creeper>(aggressor))
    {
        return false;
    }

    if (canAttack(aggressor, false))
    {
        return true;
    }

    if (mMob->getRandom()->nextInt(20) != 0)
    {
        return false;
    }

    mVillageAggressorTarget = village->getClosestBadStandingPlayer(
        boost::static_pointer_cast<LivingEntity>(mGolem->shared_from_this()));
    return canAttack(mVillageAggressorTarget.lock(), false);
}

void DefendVillageTargetGoal::start()
{
    mGolem->setTarget(mVillageAggressorTarget.lock());
    TargetGoal::start();
}

std::wstring DefendVillageTargetGoal::getName()
{
    return std::wstring(L"DefendVillageTarget");
}
