#include "game/entity/ai/JumpGoal.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/core/Dolphin.hpp"
#include "game/core/Mth.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

JumpGoal::JumpGoal(Dolphin* mob, int interval)
    : Goal(), mInterval(interval), mMob(mob), mCanJump(false)
{
}

JumpGoal::~JumpGoal()
{
}

bool JumpGoal::isObstacle()
{
    const Direction* direction = mMob->getDirection();
    BlockPos position(mMob);
    Level* level = mMob->mLevel;
    if (!level->getBlockState(position.offset(direction->stepX * 2, 0, direction->stepZ * 2))->isSolidRender())
    {
        return false;
    }
    if (level->getBlockState(position.offset(direction->stepX, 0, direction->stepZ))->getMaterial() != Material::WATER)
    {
        return false;
    }
    if (level->getBlockState(position.offset(direction->stepX, 1, direction->stepZ))->getMaterial() != Material::AIR)
    {
        return false;
    }
    if (level->getBlockState(position.offset(direction->stepX, 2, direction->stepZ))->getMaterial() != Material::AIR)
    {
        return false;
    }
    return level->getBlockState(position.offset(direction->stepX * 4, 0, direction->stepZ * 4))->getMaterial() == Material::WATER;
}

bool JumpGoal::canUse()
{
    if (mMob->getRandom()->nextInt(mInterval) != 0)
    {
        return false;
    }
    const Direction* direction = mMob->getDirection();
    BlockPos position(mMob);
    bool clear = true;
    for (int distance = 1; distance <= 5; ++distance)
    {
        if (mMob->mLevel->getBlockState(position.relative(direction, distance))->getMaterial() != Material::WATER)
        {
            clear = false;
            break;
        }
    }
    if (!isObstacle() && !clear)
    {
        return false;
    }
    return mMob->mLevel->getBlockState(position.above(2))->getMaterial() == Material::AIR;
}

bool JumpGoal::canContinueToUse()
{
    if (mMob->mYd * mMob->mYd < 0.03 && mMob->mXRot != 0.0f && std::fabs(mMob->mXRot) < 10.0f
        && mMob->getNavigation()->isDone())
    {
        return false;
    }
    return !mMob->mOnGround;
}

bool JumpGoal::canInterrupt()
{
    return false;
}

void JumpGoal::start()
{
    const Direction* direction = mMob->getDirection();
    mMob->mXd += static_cast<double>(direction->stepX) * 0.6;
    mMob->mYd += 0.7;
    mMob->mZd += static_cast<double>(direction->stepZ) * 0.6;
    mMob->getNavigation()->stop();
}

void JumpGoal::stop()
{
    mMob->mXRot = 0.0f;
}

double JumpGoal::rotlerp(float from, float to, float amount)
{
    float difference = to - from;
    while (difference < -180.0f)
    {
        difference += 360.0f;
    }
    while (difference >= 180.0f)
    {
        difference -= 360.0f;
    }
    return static_cast<double>(from + amount * difference);
}

void JumpGoal::tick()
{
    if (!mCanJump && !mMob->canAddPassenger(boost::shared_ptr<Entity>()))
    {
        mCanJump = true;
        mMob->playSound(SoundEvents::DOLPHIN_JUMP, 1.0f, 1.0f);
    }
    double verticalSpeed = mMob->mYd * mMob->mYd;
    if (verticalSpeed < 0.03 && mMob->mXRot != 0.0f)
    {
        mMob->mXRot = static_cast<float>(rotlerp(mMob->mXRot, 0.0f, 0.2f));
        return;
    }
    double horizontalSpeed = std::sqrt(mMob->mXd * mMob->mXd + mMob->mZd * mMob->mZd);
    double speed = std::sqrt(horizontalSpeed * horizontalSpeed + verticalSpeed);
    double angle = -mMob->mYd;
    int sign = (angle > 0.0 ? 1 : angle < 0.0 ? -1 : 0);
    double pitch = std::acos(Mth::clamp(horizontalSpeed / speed, -1.0, 1.0));
    mMob->mXRot = static_cast<float>(static_cast<double>(sign) * pitch * 57.2957763671875);
}

std::wstring JumpGoal::getName()
{
    return std::wstring(L"Jump_Goal");
}
