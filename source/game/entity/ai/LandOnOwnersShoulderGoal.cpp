#include "game/entity/ai/LandOnOwnersShoulderGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/ShoulderRidingEntity.hpp"
#include "game/util/AABB.hpp"

LandOnOwnersShoulderGoal::LandOnOwnersShoulderGoal(ShoulderRidingEntity* owner)
    : mOwner(owner)
{
}

LandOnOwnersShoulderGoal::~LandOnOwnersShoulderGoal()
{
}

bool LandOnOwnersShoulderGoal::canUse()
{
    boost::shared_ptr<LivingEntity> livingEntity = mOwner->getLastHurtByMob();
    if (!livingEntity)
    {
        return false;
    }

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(livingEntity);
    if (player->isSpectator() || player->isCreative() || player->isPassenger())
    {
        return false;
    }

    return !mOwner->isSitting() && mOwner->canSitOnShoulder();
}

bool LandOnOwnersShoulderGoal::canInterrupt()
{
    return !mShoulderRiding;
}

void LandOnOwnersShoulderGoal::start()
{
    mPlayer = boost::static_pointer_cast<Player>(mOwner->getLastHurtByMob());
    mShoulderRiding = false;
}

void LandOnOwnersShoulderGoal::tick()
{
    if (!mShoulderRiding && !mOwner->isSitting() && !mOwner->isLeashed() && mPlayer)
    {
        AABB* ownerBox = mOwner->mBoundingBox;
        AABB* playerBox = mPlayer->mBoundingBox;
        if (ownerBox->intersects(playerBox->mMinX,
                                 playerBox->mMinY,
                                 playerBox->mMinZ,
                                 playerBox->mMaxX,
                                 playerBox->mMaxY,
                                 playerBox->mMaxZ))
        {
            mShoulderRiding = mOwner->setEntityOnShoulder(mPlayer);
        }
    }
}

std::wstring LandOnOwnersShoulderGoal::getName()
{
    return std::wstring(L"LandOnOwnersShoulderGoal");
}
