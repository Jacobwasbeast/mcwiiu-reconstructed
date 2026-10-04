#include "game/entity/ai/WitherRandomAttackPosGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/WitherBoss.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"

WitherRandomAttackPosGoal::WitherRandomAttackPosGoal(PathfinderMob* mob, float speedModifier,
                                                     int interval, int xz, int y)
    : RandomStrollGoal(mob, (double)speedModifier, interval),
      mIsPathing(false),
      mXz(xz),
      mY(y)
{
    setRequiredControlFlags(3);
}

WitherRandomAttackPosGoal::~WitherRandomAttackPosGoal()
{
}

void WitherRandomAttackPosGoal::start()
{
    WitherBoss* wither = static_cast<WitherBoss*>(mMob);
    mIsPathing = true;
    wither->setIsPathing(true);
    wither->mFlyingSpeed *= 15.0f;
    RandomStrollGoal::start();
}

void WitherRandomAttackPosGoal::stop()
{
    WitherBoss* wither = static_cast<WitherBoss*>(mMob);
    mMob->mFlyingSpeed /= 15.0f;
    mIsPathing = false;
    wither->setWantsToMove(false);
    wither->setIsPathing(false);
    wither->setShotDelay(20);
    Goal::stop();
}

bool WitherRandomAttackPosGoal::canUse()
{
    WitherBoss* wither = static_cast<WitherBoss*>(mMob);
    if (mMob->getTarget() && !wither->isPowered() && wither->wantsToMove())
    {
        Vec3* targetPos = mMob->getTarget()->getPos(1.0f);

        if (!mIsPathing)
        {
            boost::shared_ptr<PathfinderMob> mob =
                boost::static_pointer_cast<PathfinderMob>(mMob->shared_from_this());
            Vec3* position = RandomPos::getPos(mob, mXz, mY, -1);
            if (position != NULL)
            {
                Vec3* direction = position->subtract(mMob->mX, mMob->mY, mMob->mZ);
                direction->y = 0.0;
                Vec3* target = direction->normalize()->multiply((float)mXz, (float)mXz, (float)mXz)->add(targetPos);
                target->y = mMob->mY;
                mWantedX = target->x;
                mWantedY = target->y;
                mWantedZ = target->z;
                return true;
            }
        }
    }

    return false;
}

bool WitherRandomAttackPosGoal::canContinueToUse()
{
    return RandomStrollGoal::canContinueToUse();
}
