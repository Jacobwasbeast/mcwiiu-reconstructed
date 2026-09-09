#include "game/core/WitherTargetHighestDamage.hpp"

#include <map>

#include <boost/shared_ptr.hpp>

#include "game/core/WitherBoss.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/Vec3.hpp"

WitherTargetHighestDamage::WitherTargetHighestDamage(WitherBoss* wither)
    : TargetGoal(static_cast<PathfinderMob*>(wither), false, false)
{
    setRequiredControlFlags(1);
}

WitherTargetHighestDamage::~WitherTargetHighestDamage()
{
}

boost::shared_ptr<LivingEntity> WitherTargetHighestDamage::getHighestDamageTarget()
{
    std::map<boost::shared_ptr<ServerPlayer>, int>& playerParty =
        static_cast<WitherBoss*>(mMob)->getPlayerParty();
    boost::shared_ptr<LivingEntity> target;
    int highestDamage = 0;

    std::map<boost::shared_ptr<ServerPlayer>, int>::const_iterator it;
    for (it = playerParty.begin(); it != playerParty.end(); ++it)
    {
        boost::shared_ptr<LivingEntity> player =
            boost::static_pointer_cast<LivingEntity>(it->first);
        if (it->second > highestDamage && canAttack(player, false))
        {
            highestDamage = it->second;
            target = player;
        }
    }

    return target;
}

bool WitherTargetHighestDamage::canUse()
{
    boost::shared_ptr<LivingEntity> target = getHighestDamageTarget();
    if (target && canAttack(target, false))
    {
        mTarget = target;
        return true;
    }

    return false;
}

bool WitherTargetHighestDamage::canContinueToUse()
{
    if (!TargetGoal::canContinueToUse())
        return false;

    return getHighestDamageTarget() == mTarget.lock();
}

void WitherTargetHighestDamage::start()
{
    mMob->setTarget(mTarget.lock());
}

bool WitherTargetHighestDamage::canAttack(
    boost::shared_ptr<LivingEntity> target, bool allowInvulnerablePlayers)
{
    double followDistance =
        mMob->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->getValue();
    Vec3* position = mMob->getPos(1.0f);
    double distanceSquared =
        position->subtract(target->mX, target->mY, target->mZ)->lengthSqr();

    if (distanceSquared <= followDistance * followDistance)
    {
        return TargetGoal::canAttack(mMob, target, allowInvulnerablePlayers,
                                     mMustSee);
    }

    return false;
}
