#include "game/world/generation/RemotePlayer.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/GameType.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/BlockPos.hpp"

RemotePlayer::RemotePlayer(Level* level, const std::wstring& name)
    : AbstractClientPlayer(level, name),
      mGameType(NULL)
{
    fjDerivedCtorCalls();
    mBedOffsetY = 0.25f;
    mNoPhysics = true;
    mMaxUpStep = 1.0f;
    setViewScale(10.0);
    mField0x878 = 0;
    mField0x880 = 0.0f;
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
    mBedOffsetY = 0;
    AbstractClientPlayer::tick();

    double horizontalDistance = std::sqrt((mX - mXo) * (mX - mXo)
        + (mZ - mZo) * (mZ - mZo));
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
        mYBodyRot = mYRot;
    }

    mPrevCameraYaw = mCameraYaw;
    updateSwingTime();
    double horizontalSpeed = Mth::sqrt(mXd * mXd + mZd * mZd);
    double verticalAngle = static_cast<float>(std::atan(-(mYd * 0.20000000298023224))) * 15.0f;
    if (horizontalSpeed > 0.1f)
        horizontalSpeed = 0.1f;
    double cameraYawTarget = 0.0f;
    if (mOnGround && getHealth() > 0.0f)
        cameraYawTarget = horizontalSpeed;
    if (mOnGround || getHealth() <= 0.0f)
        verticalAngle = 0.0f;
    mBob = static_cast<float>(verticalAngle - mBob) * 0.8f + mBob;
    mCameraYaw = static_cast<float>(cameraYawTarget - mCameraYaw) * 0.4f + mCameraYaw;
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
    return EntityType::REMOTE_PLAYER;
}
