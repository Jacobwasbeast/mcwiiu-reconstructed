#include "game/core/WitherTargetHighestDamage.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Vec3.hpp"

WitherTargetHighestDamage::WitherTargetHighestDamage(WitherBoss* wither)
    : TargetGoal(wither, false, false)
{
    setRequiredControlFlags(1);
}

WitherTargetHighestDamage::~WitherTargetHighestDamage()
{
    boost::shared_ptr<LivingEntity> target = mTarget.lock();
    mTarget.reset();
    target.reset();
}

boost::shared_ptr<LivingEntity> WitherTargetHighestDamage::getHighestDamageTarget()
{
    boost::shared_ptr<LivingEntity> target;
    int highestDamage = 0;

    std::map<boost::shared_ptr<ServerPlayer>, int>& party = static_cast<WitherBoss*>(mMob)->getPlayerParty();
    for (std::map<boost::shared_ptr<ServerPlayer>, int>::const_iterator it = party.begin(); it != party.end(); ++it)
    {
        int damage = it->second;
        boost::shared_ptr<ServerPlayer> player = it->first;
        if (damage > highestDamage && canAttack(player, false))
        {
            highestDamage = damage;
            target = player;
        }
    }
    return target;
}

bool WitherTargetHighestDamage::canUse()
{
    boost::shared_ptr<LivingEntity> target = getHighestDamageTarget();
    if (!target) return false;
    if (!canAttack(target, false)) return false;
    mTarget = target;
    return true;
}

bool WitherTargetHighestDamage::canContinueToUse()
{
    if (!TargetGoal::canContinueToUse()) return false;
    return getHighestDamageTarget() == mTarget.lock();
}

void WitherTargetHighestDamage::start()
{
    boost::shared_ptr<LivingEntity> target = mTarget.lock();
    static_cast<WitherBoss*>(mMob)->setTarget(target);
    target.reset();
}

bool WitherTargetHighestDamage::canAttack(boost::shared_ptr<LivingEntity> target,
                                          bool allowInvulnerablePlayers)
{
    const float partialTicks = 1.0f;
    Vec3* targetPosition = target->getPos(partialTicks);
    Vec3* witherPosition = mMob->getPos(partialTicks);
    Vec3* delta = targetPosition->subtract(witherPosition);
    double followDistance = getFollowDistance();
    if (delta->lengthSqr() > followDistance * followDistance)
        return false;
    return TargetGoal::canAttack(mMob, target, allowInvulnerablePlayers, mMustSee);
}
