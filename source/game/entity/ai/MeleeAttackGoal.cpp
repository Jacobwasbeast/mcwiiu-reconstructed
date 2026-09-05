#include "game/entity/ai/MeleeAttackGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Difficulty.hpp"
#include "game/core/Path.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/core/Sensing.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

MeleeAttackGoal::MeleeAttackGoal(PathfinderMob* mob, double speedModifier, bool followingTargetEvenIfNotSeen)
    : mAttackInterval(20)
{
    _init(mob, speedModifier, followingTargetEvenIfNotSeen);
}

MeleeAttackGoal::~MeleeAttackGoal()
{
    if (mPath != NULL)
        delete mPath;
}

void MeleeAttackGoal::_init(PathfinderMob* mob, double speedModifier, bool followingTargetEvenIfNotSeen)
{
    mMob = mob;
    mLevel = mob->mLevel;
    mSpeedModifier = speedModifier;
    mFollowingTargetEvenIfNotSeen = followingTargetEvenIfNotSeen;
    setRequiredControlFlags(3);
    mTicksUntilNextAttack = 0;
    mPath = NULL;
    mTicksUntilNextPathRecalculation = 0;
    mTargetX = 0.0;
    mTargetY = 0.0;
    mTargetZ = 0.0;
}

bool MeleeAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target || !target->isAlive())
        return false;

    delete mPath;
    mPath = mMob->getNavigation()->createPath(target);
    if (mPath != NULL)
        return true;

    if (getAttackReachSqr(target) < mMob->distanceToSqr(target->mX, target->mBoundingBox->mMinY, target->mZ))
    {
        if ((target->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER
            && mMob->mLevel->getDifficulty() == Difficulty::PEACEFUL)
            mMob->setTarget(boost::shared_ptr<LivingEntity>());
        return false;
    }
    return true;
}

bool MeleeAttackGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target || !target->isAlive())
        return false;

    if (!mFollowingTargetEvenIfNotSeen)
        return !mMob->getNavigation()->isDone();

    if (mMob->isWithinRestriction(BlockPos(target)))
    {
        if ((target->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(target);
            if (player->isSpectator() || player->isCreative()
                || mMob->mLevel->getDifficulty() == Difficulty::PEACEFUL)
                return false;
        }
        return true;
    }
    return false;
}

void MeleeAttackGoal::start()
{
    mMob->getNavigation()->moveTo(mPath, mSpeedModifier);
    mPath = NULL;
    mTicksUntilNextPathRecalculation = 0;
}

void MeleeAttackGoal::stop()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (target && (target->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(target);
        if (player->isSpectator() || player->isCreative())
            mMob->setTarget(boost::shared_ptr<LivingEntity>());
    }

    mMob->getNavigation()->stop();
}

void MeleeAttackGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    static_cast<LookControl*>(mMob->getLookControl())->setLookAt(target, 30.0f, 30.0f);
    double distance = mMob->distanceToSqr(target->mX, target->mBoundingBox->mMinY, target->mZ);
    bool canSeeTarget = true;
    --mTicksUntilNextPathRecalculation;

    if (!mFollowingTargetEvenIfNotSeen)
        canSeeTarget = mMob->getSensing()->canSee(target);

    if (canSeeTarget && mTicksUntilNextPathRecalculation <= 0)
    {
        bool recalculatePath = true;
        if (mTargetX != 0.0 || mTargetY != 0.0 || mTargetZ == 0.0)
        {
            double targetDelta = target->distanceToSqr(mTargetX, mTargetY, mTargetZ);
            if (targetDelta < 1.0 && mMob->getRandom()->nextFloat() >= 0.05f)
                recalculatePath = false;
        }

        if (recalculatePath)
        {
            mTargetX = target->mX;
            mTargetY = target->mBoundingBox->mMinY;
            mTargetZ = target->mZ;

            int pathRecalculationDelay = mMob->getRandom()->nextInt(7);
            if (distance <= 256.0)
                mTicksUntilNextPathRecalculation = pathRecalculationDelay + 4;
            else if (distance <= 1024.0)
                mTicksUntilNextPathRecalculation = pathRecalculationDelay + 9;
            else
                mTicksUntilNextPathRecalculation = pathRecalculationDelay + 14;

            if (!mMob->getNavigation()->moveTo(target, mSpeedModifier))
                mTicksUntilNextPathRecalculation += 15;
        }
    }

    mTicksUntilNextAttack = mTicksUntilNextAttack - 1;
    if (mTicksUntilNextAttack < 0)
        mTicksUntilNextAttack = 0;
    checkAndPerformAttack(target, distance);
}

void MeleeAttackGoal::checkAndPerformAttack(boost::shared_ptr<LivingEntity> target, double distance)
{
    if (distance <= getAttackReachSqr(target) && mTicksUntilNextAttack <= 0)
    {
        mTicksUntilNextAttack = mAttackInterval;
        mMob->swing(InteractionHand::eInteractionHand_MainHand);
        mMob->doHurtTarget(target);
    }
}

double MeleeAttackGoal::getAttackReachSqr(boost::shared_ptr<LivingEntity> target)
{
    Entity* reachEntity = mMob;
    if (Entity::instanceof(mMob->getVehicle(), ENTITY_TYPE_MOB))
        reachEntity = mMob->getVehicle().get();
    double width = reachEntity->mBbWidth + reachEntity->mBbWidth;
    return width * width + target->mBbWidth;
}

void MeleeAttackGoal::setLevel(Level* level)
{
    mLevel = level;
}

std::wstring MeleeAttackGoal::getName()
{
    return std::wstring(L"MeleeAttack");
}
