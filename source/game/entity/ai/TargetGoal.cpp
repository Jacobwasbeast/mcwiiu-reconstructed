#include "game/entity/ai/TargetGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/AttributeInstance.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/Path.hpp"
#include "game/core/Sensing.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/WitherSkeleton.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

TargetGoal::TargetGoal(PathfinderMob* mob, bool mustSee)
    : Goal()
{
    _init(mob, mustSee, false);
}

TargetGoal::TargetGoal(PathfinderMob* mob, bool mustSee, bool mustReach)
    : Goal()
{
    _init(mob, mustSee, mustReach);
}

TargetGoal::~TargetGoal()
{
}

void TargetGoal::_init(PathfinderMob* mob, bool mustSee, bool mustReach)
{
    mMob = mob;
    mMustSee = mustSee;
    mMustReach = mustReach;
    mCanReach = 0;
    mTargetSearchDelay = 0;
    mUnseenTicks = 0;
    mTarget.reset();
    mUnseenMemoryTicks = 60;
}

bool TargetGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target || !target->isAlive())
        return false;

    if ((mMob->GetType() & 0x000000005D540442ULL) == 0x000000005D540442ULL)
    {
        WitherSkeleton* wither = static_cast<WitherSkeleton*>(mMob);
        boost::shared_ptr<LivingEntity> parent = wither->getWitherParent();
        if (parent && parent->getId() == target->getId())
            return false;
    }

    if (mMob->distanceToSqr(target) > getFollowDistance() * getFollowDistance())
        return false;

    if (mMustSee)
    {
        if (mMob->getSensing()->canSee(target))
            mUnseenTicks = 0;
        else
            ++mUnseenTicks;
        if (mUnseenTicks > mUnseenMemoryTicks)
            return false;
    }

    if ((target->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        if (target->isCreative() || mMob->mLevel->getDifficulty() == Difficulty::PEACEFUL)
            return false;
    }

    return true;
}

double TargetGoal::getFollowDistance()
{
    AttributeInstance* attribute = mMob->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE);
    if (attribute == NULL)
        return 16.0;
    return attribute->getValue();
}

void TargetGoal::start()
{
    mCanReach = 0;
    mTargetSearchDelay = 0;
    mUnseenTicks = 0;
}

void TargetGoal::stop()
{
    mMob->setTarget(boost::shared_ptr<LivingEntity>());
    mTarget.reset();
}

bool TargetGoal::canAttack(Mob* mob, boost::shared_ptr<LivingEntity> target,
                           bool allowInvulnerablePlayers, bool requireSight)
{
    if (!target || target.get() == mob || !target->isAlive())
        return false;

    if (mob->isAlliedTo(target))
        return false;

    if (!target->isAttackable())
        return false;

    if ((target->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER
        && !allowInvulnerablePlayers)
    {
        if (target->isCreative() || mob->mLevel->getDifficulty() == Difficulty::PEACEFUL)
            return false;
    }

    if (requireSight && !mob->getSensing()->canSee(target))
        return false;

    return true;
}

bool TargetGoal::canReach(boost::shared_ptr<LivingEntity> target)
{
    mTargetSearchDelay = mMob->getRandom()->nextInt(5) + 10;
    Path* path = mMob->getNavigation()->createPath(target);
    if (path == NULL)
        return false;

    Node* last = path->last();
    if (last == NULL)
        return false;

    int x = (int)target->mX;
    int z = (int)target->mZ;
    bool result = ((last->x - x) * (last->x - x) + (last->z - z) * (last->z - z)) <= 2.25;
    delete path;
    return result;
}

bool TargetGoal::canAttack(boost::shared_ptr<LivingEntity> target, bool allowInvulnerablePlayers)
{
    if (!canAttack(mMob, target, allowInvulnerablePlayers, mMustSee))
        return false;

    BlockPos pos((int)target->mX, (int)target->mY, (int)target->mZ);
    if (!mMob->isWithinRestriction(pos))
        return false;

    if (!mMustReach)
        return true;

    --mTargetSearchDelay;
    if (mTargetSearchDelay > 0 && mCanReach != 0)
        return mCanReach == 1;

    mCanReach = canReach(target) ? 1 : 2;
    return mCanReach == 1;
}

void TargetGoal::setUnseenMemoryTicks(int ticks)
{
    mUnseenMemoryTicks = ticks;
}

std::wstring TargetGoal::getName()
{
    return std::wstring(L"TargetGoal");
}
