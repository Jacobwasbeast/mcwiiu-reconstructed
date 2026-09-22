#include "game/core/WitherTargetHighestDamage.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/entity/mob/LivingEntity.hpp"
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
    WitherBoss* wither = static_cast<WitherBoss*>(mMob);
    wither->getPlayerParty();
    WitherBoss* boss = static_cast<WitherBoss*>(mMob);
    boost::shared_ptr<LivingEntity> candidate = boss->Mob::getTarget();
    return candidate;
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
    boost::shared_ptr<LivingEntity> target = getHighestDamageTarget();
    boost::shared_ptr<LivingEntity> selected = mTarget.lock();
    if (!target || !selected) return false;
    return selected.get() == target.get();
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
    return TargetGoal::canAttack(mMob, target, allowInvulnerablePlayers, false);
}
