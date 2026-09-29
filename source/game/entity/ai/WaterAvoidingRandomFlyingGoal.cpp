#include "game/entity/ai/WaterAvoidingRandomFlyingGoal.hpp"

#include <boost/shared_ptr.hpp>
#include <cmath>

#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/RandomPos.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"

WaterAvoidingRandomFlyingGoal::WaterAvoidingRandomFlyingGoal(PathfinderMob* mob, double speedModifier)
    : WaterAvoidingRandomStrollGoal(mob, speedModifier)
{
}

WaterAvoidingRandomFlyingGoal::~WaterAvoidingRandomFlyingGoal()
{
}

Vec3* WaterAvoidingRandomFlyingGoal::getTreePos()
{
    BlockPos origin(mMob);
    MutableBlockPos candidate(0, 0, 0);
    MutableBlockPos below(0, 0, 0);

    const int minX = static_cast<int>(std::floor(mMob->mX - 3.0));
    const int maxX = static_cast<int>(std::floor(mMob->mX + 3.0));
    const int minY = static_cast<int>(std::floor(mMob->mY - 6.0));
    const int maxY = static_cast<int>(std::floor(mMob->mY + 6.0));
    const int minZ = static_cast<int>(std::floor(mMob->mZ - 3.0));
    const int maxZ = static_cast<int>(std::floor(mMob->mZ + 3.0));

    for (int x = minX; x <= maxX; ++x)
    {
        candidate.setX(x);
        for (int y = minY; y <= maxY; ++y)
        {
            candidate.setY(y);
            for (int z = minZ; z <= maxZ; ++z)
            {
                candidate.setZ(z);
                if (origin.equals(candidate))
                {
                    continue;
                }

                below.set(candidate);
                below.move(Direction::DOWN);
                Block* block = mMob->mLevel->getBlockState(below)->getBlock();
                if ((dynamic_cast<LeafBlock*>(block) != NULL
                     || block == Blocks::log || block == Blocks::log2)
                    && mMob->mLevel->isEmptyBlock(candidate)
                    && mMob->mLevel->isEmptyBlock(candidate.above()))
                {
                    return Vec3::newTemp(candidate.x, candidate.y, candidate.z);
                }
            }
        }
    }

    return NULL;
}

Vec3* WaterAvoidingRandomFlyingGoal::getPosition()
{
    boost::shared_ptr<PathfinderMob> mob = boost::static_pointer_cast<PathfinderMob>(mMob->shared_from_this());

    if (mMob->isInWaterOrBubble() || mMob->isInLava())
    {
        Vec3* position = RandomPos::getLandPos(mob, 15, 15, -1);
        if (position != NULL)
        {
            return position;
        }
    }

    if (mMob->getRandom()->nextFloat() < 0.001f)
    {
        Vec3* position = getTreePos();
        if (position != NULL)
        {
            return position;
        }
    }

    return WaterAvoidingRandomStrollGoal::getPosition();
}
