#include "game/entity/ai/BegGoal.hpp"

#include "game/core/InteractionHand.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/Wolf.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"

BegGoal::BegGoal(Wolf* wolf, float distance)
    : mWolf(wolf), mLevel(wolf->mLevel), mDistance(distance), mTime(0)
{
    setRequiredControlFlags(2);
}

BegGoal::~BegGoal()
{
}

bool BegGoal::playerHoldingInteresting(boost::shared_ptr<Player> player)
{
    for (int hand = InteractionHand::eInteractionHand_MainHand;
         hand <= InteractionHand::eInteractionHand_OffHand; ++hand)
    {
        if (player->getItemInHand((InteractionHand::EInteractionHand)hand)->getItem() == Items::bone)
        {
            return true;
        }
    }

    return false;
}

bool BegGoal::canUse()
{
    if (mWolf->isTame())
    {
        return false;
    }

    mPlayer = mLevel->getNearestPlayer(mWolf->shared_from_this(), mDistance, -1.0);
    return mPlayer != NULL && playerHoldingInteresting(mPlayer);
}

bool BegGoal::canContinueToUse()
{
    if (mPlayer == NULL || !mPlayer->isAlive())
    {
        return false;
    }

    if (mWolf->distanceToSqr(mPlayer) > (double)(mDistance * mDistance))
    {
        return false;
    }

    return mTime <= 0 || playerHoldingInteresting(mPlayer);
}

void BegGoal::start()
{
    mWolf->setIsInterested(true);
    mTime = mWolf->getRandom()->nextInt(40) + 40;
}

void BegGoal::stop()
{
    mWolf->setIsInterested(false);
    mPlayer.reset();
    mTime = 0;
}

void BegGoal::tick()
{
    mWolf->getLookControl()->setLookAt(mPlayer->mX, mPlayer->mY + mPlayer->getEyeHeight(),
                                       mPlayer->mZ, 10.0f, mWolf->getMaxHeadXRot());
    --mTime;
}

void BegGoal::setLevel(Level* level)
{
    mLevel = level;
}

std::wstring BegGoal::getName()
{
    return std::wstring(L"Beg");
}
