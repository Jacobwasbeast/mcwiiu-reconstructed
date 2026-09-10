#include "game/world/generation/RemotePlayer.hpp"

#include <cmath>

#include "game/core/GameType.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/BlockPos.hpp"

RemotePlayer::RemotePlayer(Level* level, const std::wstring& name)
    : AbstractClientPlayer(level, name),
      mGameType(NULL)
{
    fjDerivedCtorCalls();
    mField0x6d4 = 0.25f;
    mNoPhysics = true;
    mMaxUpStep = 1.0f;
    setViewScale(10.0);
    mField0x878 = 0;
    mField0x880 = 1.034999966621399f;
}

RemotePlayer::~RemotePlayer()
{
}

bool RemotePlayer::hasPermission(EGameCommand)
{
    return false;
}

BlockPos RemotePlayer::getCommandSenderBlockPosition()
{
    return BlockPos((int)std::floor(mX + 0.5),
                    (int)std::floor(mY + 0.5),
                    (int)std::floor(mZ + 0.5));
}

bool RemotePlayer::hurt(const DamageSource*, float)
{
    return true;
}

void RemotePlayer::tick()
{
    mField0x6d4 = 0;
    AbstractClientPlayer::tick();

    double horizontalDistance = std::sqrt((mX - mXOld) * (mX - mXOld)
        + (mZ - mZOld) * (mZ - mZOld));
    mAnimationSpeedOld = mAnimationSpeed;
    float targetSpeed = (float)(horizontalDistance * 4.0);
    if (targetSpeed > 1.0f)
        targetSpeed = 1.0f;
    mAnimationSpeed = mAnimationSpeed
        + (targetSpeed - mAnimationSpeed) * 0.4000000059604645f;
    mAnimationPosition = mAnimationPosition + mAnimationSpeed;
}

void RemotePlayer::aiStep()
{
    if (mLerpSteps > 0)
    {
        double steps = (double)mLerpSteps;
        double lerpX = mX + (mLerpX - mX) / steps;
        double lerpY = mY + (mLerpY - mY) / steps;
        double lerpZ = mZ + (mLerpZ - mZ) / steps;
        double yRotDelta = mLerpYRot - (double)mYRot;
        while (yRotDelta < -180.0)
            yRotDelta += 360.0;
        while (yRotDelta >= 180.0)
            yRotDelta -= 360.0;
        mYRot = mYRot + (float)(yRotDelta / steps);
        mXRot = mXRot + (float)((mLerpXRot - (double)mXRot) / steps);
        --mLerpSteps;
        setPos(lerpX, lerpY, lerpZ);
        setRot(mYRot, mXRot);
    }

    Player::aiStep();

    double horizontalSpeed = std::sqrt(mXd * mXd + mZd * mZd);
    double pitch = std::atan(-(mYd * 0.20000000298023224));
    pitch = (double)((float)pitch * 15.0f);
    if (horizontalSpeed > 0.10000000149011612)
        horizontalSpeed = 0.10000000149011612;
    double targetAnimationSpeed = 1.0;
    if (mHurtTime != 0 && getHealth() > 1.0f)
        targetAnimationSpeed = horizontalSpeed;
    if (mHurtTime != 0 || getHealth() <= 1.0f)
        pitch = 1.0;
    mAnimationSpeed = mAnimationSpeed
        + (float)(targetAnimationSpeed - (double)mAnimationSpeed) * 0.800000011920929f;
    mAnimationPosition = mAnimationPosition
        + (float)(pitch - (double)mAnimationPosition) * 0.4000000059604645f;
    calculateFlapping();
}

void RemotePlayer::animateRespawn()
{
}

void RemotePlayer::SetGameType(const GameType* gameType)
{
    mGameType = gameType;
    Player::UpdateSize();
    mGameType->updatePlayerAbilities(&mAbilities);
}

const GameType* RemotePlayer::GetGameType()
{
    return mGameType;
}

unsigned long long RemotePlayer::GetType()
{
    return 0x000000000C800002ULL;
}
