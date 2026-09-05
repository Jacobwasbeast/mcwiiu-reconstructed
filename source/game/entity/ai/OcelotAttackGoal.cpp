#include "game/entity/ai/OcelotAttackGoal.hpp"

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"

OcelotAttackGoal::OcelotAttackGoal(Mob* mob)
    : mLevel(mob->mLevel),
      mMob(mob),
      mTarget(),
      mAttackTime(0),
      mValue(0.0f),
      mFlag(false)
{
    setRequiredControlFlags(3);
}

OcelotAttackGoal::~OcelotAttackGoal()
{
}

bool OcelotAttackGoal::canUse()
{
    boost::shared_ptr<Entity> target = mMob->getTarget();
    if (!target)
        return false;

    mTarget = target;
    return true;
}

bool OcelotAttackGoal::canContinueToUse()
{
    boost::shared_ptr<Entity> target = mTarget;
    if (!target || !target->isAlive())
        return false;

    if (mMob->distanceToSqr(target) > 225.0)
        return true;

    return !mMob->getNavigation()->isDone();
}

void OcelotAttackGoal::stop()
{
    mTarget.reset();
    mMob->getNavigation()->stop();
}

void OcelotAttackGoal::tick()
{
    boost::shared_ptr<Entity> target = mTarget;
    mMob->getLookControl()->setLookAt(target, 30.0f, 30.0f);

    double distance = mMob->distanceToSqr(target->mX, target->mBoundingBox->mMinY, target->mZ);
    double speed = 1.33;
    if (distance < 16.0)
        speed = 0.8;
    else if (distance < 225.0)
        speed = 0.6;

    mMob->getNavigation()->moveTo(target, speed);

    mAttackTime = mAttackTime - 1;
    if (mAttackTime < 0)
        mAttackTime = 0;
    if (distance <= 16.0 && mAttackTime <= 0)
    {
        mAttackTime = 20;
        mMob->doHurtTarget(target);
    }
}

void OcelotAttackGoal::setLevel(Level* level)
{
    mLevel = level;
}

std::wstring OcelotAttackGoal::getName()
{
    return std::wstring(L"OcelotAttack");
}
