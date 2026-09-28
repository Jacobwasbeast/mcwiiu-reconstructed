#include "game/entity/ai/MoveToWaterGoal.hpp"

#include <cmath>

#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

MoveToWaterGoal::MoveToWaterGoal(PathfinderMob* mob, double speedModifier,
                                 int searchRange, int verticalSearchStart,
                                 double maxStayDistance)
    : MoveToBlockGoal(mob, speedModifier, searchRange, verticalSearchStart, 1.0),
      mSpeedModifierSquared(speedModifier * speedModifier),
      mEntity(mob)
{
}

MoveToWaterGoal::~MoveToWaterGoal()
{
}

bool MoveToWaterGoal::canUse()
{
    BlockPos pos(mEntity);
    if (mEntity->mLevel->getBlockState(pos)->getMaterial() != Material::WATER)
        return MoveToBlockGoal::canUse();
    return false;
}

bool MoveToWaterGoal::canContinueToUse()
{
    BlockPos pos(mEntity);
    if (mEntity->mLevel->getBlockState(pos)->getMaterial() != Material::WATER)
        return MoveToBlockGoal::canContinueToUse();
    return false;
}

void MoveToWaterGoal::stop()
{
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation != NULL)
        navigation->stop();
    else
        mMob->getMoveControl()->wait();
}

bool MoveToWaterGoal::isValidTarget(Level*, const BlockPos& pos)
{
    int radius = (int)std::floor(mEntity->mY);
    for (int x = 0; x <= radius; ++x)
    {
        for (int z = 0; z <= radius; ++z)
        {
            BlockPos candidate = pos + BlockPos(x, 0, z);
            if (mEntity->mLevel->getBlockState(candidate.above())->getMaterial() != Material::WATER)
                return false;
            if (mEntity->mLevel->getBlockState(candidate.above(2))->getMaterial() != Material::AIR)
                return false;
        }
    }

    mBlockPos = pos;
    return true;
}

void MoveToWaterGoal::tick()
{
    BlockPos above = mBlockPos.above();
    if (mEntity->distanceSqrToBlockPosCenter(above) <= mSpeedModifierSquared)
    {
        mReachedTarget = true;
        --mTicksStayingStill;
    }
    else
    {
        ++mTicksStayingStill;
        mReachedTarget = false;
        if (mTicksStayingStill % 40 == 0)
        {
            mMob->getNavigation()->moveTo((double)mBlockPos.x + 0.5,
                                          (double)mBlockPos.y + (double)yOff(),
                                          (double)mBlockPos.z + 0.5,
                                          mSpeedModifier);
        }
    }
}

std::wstring MoveToWaterGoal::getName()
{
    return std::wstring(L"Move_To_Water_Goal");
}

int MoveToWaterGoal::GetNextStartTick(PathfinderMob*)
{
    return 20;
}
