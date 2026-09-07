#include "game/entity/ai/BreakDoorGoal.hpp"

#include "game/core/Difficulty.hpp"
#include "game/core/GameRules.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/DoorBlock.hpp"

BreakDoorGoal::BreakDoorGoal(Mob* mob)
    : DoorInteractGoal(mob),
      mBreakingTime(0),
      mLastBreakProgress(-1)
{
}

BreakDoorGoal::~BreakDoorGoal()
{
}

bool BreakDoorGoal::canUse()
{
    return DoorInteractGoal::canUse()
        && mMob->mLevel->getGameRules()->getBoolean(1)
        && !DoorBlock::isOpen(mMob->mLevel, mDoorPos);
}

void BreakDoorGoal::start()
{
    DoorInteractGoal::start();
    mBreakingTime = 0;
}

void BreakDoorGoal::stop()
{
    Goal::stop();
    mMob->mLevel->destroyBlockProgress(mMob->getId(), mDoorPos, -1);
}

bool BreakDoorGoal::canContinueToUse()
{
    return mBreakingTime < 241
        && !DoorBlock::isOpen(mMob->mLevel, mDoorPos)
        && mMob->distanceToSqr(mDoorPos) < 4.0;
}

void BreakDoorGoal::tick()
{
    DoorInteractGoal::tick();

    if (mMob->getRandom()->nextInt(20) == 0)
        mMob->mLevel->levelEvent(1019, mDoorPos, 0);

    ++mBreakingTime;
    int progress = (int)(((float)mBreakingTime / 240.0f) * 10.0f);
    if (progress != mLastBreakProgress)
    {
        mMob->mLevel->destroyBlockProgress(mMob->getId(), mDoorPos, progress);
        mLastBreakProgress = progress;
    }

    if (mBreakingTime == 240 && mMob->mLevel->getDifficulty() == Difficulty::HARD)
    {
        mMob->mLevel->removeBlock(mDoorPos);
        mMob->mLevel->levelEvent(1021, mDoorPos, 0);
        mMob->mLevel->levelEvent(2001, mDoorPos, Block::getId(mDoorBlock));
    }
}

std::wstring BreakDoorGoal::getName()
{
    return std::wstring(L"BreakDoor");
}
