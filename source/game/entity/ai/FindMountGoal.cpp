#include "game/entity/ai/FindMountGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"

FindMountGoal::FindMountGoal(Mob& mob, float targetDistance, int startDelay,
                             bool avoidWater, bool targetNeeded, float mountDistance)
    : mTarget(), mTargetDistance(targetDistance),
      mStartCounter(0), mStartDelay(startDelay), mAvoidWater(avoidWater),
      mTargetNeeded(targetNeeded), mMountDistance(mountDistance), mMob(&mob)
{
    setRequiredControlFlags(3);
}

FindMountGoal::~FindMountGoal()
{
    mTarget.reset();
}

bool FindMountGoal::canUse()
{
    if (!static_cast<Zombie*>(mMob)->isJockey())
        return false;

    if (!mMob->getNavigation() || mMob->isPassenger())
        return false;

    boost::shared_ptr<LivingEntity> attackTarget = mMob->getTarget();
    if (mTargetNeeded && (!attackTarget || mMob->isPassenger()))
        return false;

    if (mStartCounter < mStartDelay)
    {
        ++mStartCounter;
        return false;
    }

    AABB* searchBox = mMob->mBoundingBox->grow((double)mTargetDistance);
    std::vector<boost::shared_ptr<Entity> >* entities =
        mMob->mLevel->getEntitiesOfClass(typeid(Mob), searchBox);

    const unsigned long long mountTypes[] = {
        0x0D680004ULL, 0x5D560041ULL, 0x2D6A0000ULL, 0x2D698001ULL,
        0x2D690001ULL, 0x5D550043ULL, 0x2D6A0001ULL, 0x2D698002ULL,
        0x2D6C0001ULL, 0x2D680003ULL, 0x2D680002ULL, 0x2D690002ULL,
        0x5D560040ULL, 0x0D6C0002ULL, 0x5D550040ULL, 0x2D690003ULL,
        0x5D550042ULL
    };
    double nearestDistance = -1.0;
    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> candidate = *it;
        unsigned long long type = candidate->GetType();
        if ((type & 0x0D000000ULL) != 0x0D000000ULL || candidate->getId() == mMob->getId())
            continue;

        boost::shared_ptr<Mob> mount = boost::static_pointer_cast<Mob>(candidate);
        if (!mount->isAlive() || !mount->canAddPassenger(mMob->shared_from_this()) || mount->isSleeping())
            continue;
        if ((type & 0x0D6C0000ULL) == 0x0D6C0000ULL &&
            boost::static_pointer_cast<TamableAnimal>(mount)->isTame())
            continue;
        if ((type & 0x0D690000ULL) == 0x0D690000ULL &&
            boost::static_pointer_cast<AbstractHorse>(mount)->isTamed())
            continue;
        if (mount->isBaby() || (mAvoidWater && mount->isInWater()))
            continue;

        bool mountType = false;
        for (unsigned int index = 0; index < sizeof(mountTypes) / sizeof(mountTypes[0]); ++index)
        {
            if (type == mountTypes[index])
            {
                mountType = true;
                break;
            }
        }
        if (!mountType)
            continue;

        double distance = mount->distanceToSqr(mMob->shared_from_this());
        if (nearestDistance < 0.0 || distance < nearestDistance)
        {
            mTarget = mount;
            nearestDistance = distance;
        }
    }

    delete entities;
    boost::shared_ptr<Entity> target = mTarget.lock();
    if (!target)
        return false;

    mTimeToRecalcPath = mMob->mLevel->mRandom->nextInt(7) + 4;
    return mMob->getNavigation()->moveTo(target, 1.0);
}

bool FindMountGoal::canContinueToUse()
{
    boost::shared_ptr<Entity> target = mTarget.lock();
    if (!target || !target->isAlive() || mMob->isPassenger() ||
        !target->canAddPassenger(mMob->shared_from_this()))
    {
        mMob->setTarget(boost::shared_ptr<LivingEntity>());
        return false;
    }
    return true;
}

void FindMountGoal::start()
{
}

void FindMountGoal::stop()
{
    mMob->getNavigation()->stop();
    mTarget.reset();
    mStartCounter = 0;
}

double FindMountGoal::getAttackReachSqr()
{
    boost::shared_ptr<Entity> target = mTarget.lock();
    float reach = mMob->mBbWidth + mMob->mBbWidth + target->mBbWidth + 2.0f;
    return (double)(reach * reach);
}

void FindMountGoal::tick()
{
    boost::shared_ptr<Entity> target = mTarget.lock();
    if (!target)
        return;

    mMob->getLookControl()->setLookAt(target, 30.0f, 30.0f);
    double mountRange = (double)mMountDistance;
    if (mountRange < 0.0)
        mountRange = getAttackReachSqr();

    BlockPos ridingPos(target->getPos(0.0f)->add(0.0, target->getRidingHeight(), 0.0));
    if (mMob != target.get() && mMob->distanceToSqr(ridingPos) < mountRange && target->isAlive())
    {
        static_cast<Entity*>(mMob)->startRiding(target);
        stop();
        return;
    }

    --mTimeToRecalcPath;
    if (mTimeToRecalcPath < 1)
    {
        mTimeToRecalcPath = mMob->mLevel->mRandom->nextInt(7) + 4;
        mMob->getNavigation()->moveTo(target, 1.0);
    }
}

std::wstring FindMountGoal::getName()
{
    return std::wstring(L"FindMountGoal");
}
