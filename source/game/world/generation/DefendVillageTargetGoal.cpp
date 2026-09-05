#include "game/world/generation/DefendVillageTargetGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/VillagerGolem.hpp"

class Village;

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
        return false;

    boost::shared_ptr<LivingEntity> golem =
        boost::dynamic_pointer_cast<LivingEntity>(mGolem->shared_from_this());
    mTarget = village->getClosestAggressor(golem);

    if (mTarget && mTarget->GetType() == 0x5D540041ULL)
    {
        mTarget.reset();
        return false;
    }

    if (mTarget && canAttack(mTarget, false))
        return true;

    if (mGolem->getRandom()->nextInt(20) != 0)
        return false;

    mTarget = village->getClosestBadStandingPlayer(golem);
    return mTarget && canAttack(mTarget, false);
}

void DefendVillageTargetGoal::start()
{
    mGolem->setTarget(mTarget);
    TargetGoal::start();
}

std::wstring DefendVillageTargetGoal::getName()
{
    return std::wstring(L"DefendVillageTarget");
}
