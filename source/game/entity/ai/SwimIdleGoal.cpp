#include "game/entity/ai/SwimIdleGoal.hpp"

#include <string>

#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"

SwimIdleGoal::SwimIdleGoal(Mob* mob)
    : Goal(), mMob(mob), mTicks(0), mStartPos(NULL)
{
    setRequiredControlFlags(1);
}

SwimIdleGoal::~SwimIdleGoal()
{
}

bool SwimIdleGoal::canUse()
{
    if (mMob->followsLeader())
    {
        return false;
    }

    if (mMob->getNavigation()->isRiverFollowing())
    {
        return false;
    }

    if (!mMob->getNavigation()->isDone())
    {
        return false;
    }

    if (!mMob->isInWater())
    {
        return false;
    }

    return mMob->getRandom()->nextInt(10) == 0;
}

bool SwimIdleGoal::canContinueToUse()
{
    return mMob->isInWater() && mTicks < 100;
}

void SwimIdleGoal::start()
{
    mTicks = 0;
    mStartPos = mMob->getPos(1.0f);
    Goal::start();
}

void SwimIdleGoal::tick()
{
    ++mTicks;
    Goal::tick();

    mMob->mXd = 0.0;
    mMob->mYd = 0.0;
    mMob->mZd = 0.0;

    const unsigned long long type = mMob->GetType();
    if ((type & 0x0D502000ULL) != 0x0D502000ULL
        && (type & 0x0D602000ULL) != 0x0D602000ULL)
    {
        return;
    }

    if (mMob->mSpeed <= 1.0f)
    {
        return;
    }

    const int id = mMob->getId();
    const float phase = static_cast<float>(id % 100);
    const float angle = (static_cast<float>(mMob->mTickCount) + phase) * mMob->mMovementSpeedScale;
    const float swim = Mth::sin(angle) * mMob->mSpeed;
    const float yaw = mMob->mYRot * 0.01745329238474369f;
    const float cosYaw = Mth::cos(yaw);
    const float sinYaw = Mth::sin(yaw);

    mMob->mXd += static_cast<double>(swim * cosYaw);
    mMob->mZd += static_cast<double>(swim * sinYaw);

    const float bob = Mth::sin((static_cast<float>(mMob->mTickCount) + phase) * 0.75f);
    mMob->mYd += static_cast<double>(bob * 0.05f * (sinYaw + cosYaw) * 0.5f);
}

std::wstring SwimIdleGoal::getName()
{
    return std::wstring(L"swim_idle");
}
