#include "game/entity/ai/RandomSwimmingGoal.hpp"

#include <boost/shared_ptr.hpp>
#include <cmath>

#include "game/util/Math.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/MutableBlockPos.hpp"

class PathfinderMob : public Entity
{
public:
    PathNavigation* getNavigation();
};

class RandomPos
{
public:
    static Vec3* getPos(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, int wanderingQuadrant);
};

RandomSwimmingGoal::RandomSwimmingGoal(PathfinderMob* mob, double speedModifier, int interval, int xz, int y)
    : RandomStrollGoal(mob, speedModifier, interval), mXz(xz), mY(y)
{
}

RandomSwimmingGoal::~RandomSwimmingGoal()
{
}

void RandomSwimmingGoal::getWaterHeights(MutableBlockPos pos, int& topY, float& waterY, float& minY, float& maxY)
{
    Entity* entity = (Entity*)mMob;
    Level* level = entity->mLevel;
    topY = level->getAboveTopSolidBlock(pos, false, false);
    waterY = (float)topY;

    const float step = 1.0f;
    const float lowerOffset = 20.0f;
    const float upperOffset = 1.5f;
    const float height = entity->mBbHeight;
    const float yOffset = (float)mY;
    const float lowerBound = height - yOffset;
    const float upperBound = height + yOffset;

    if (waterY < (float)level->getMaxBuildHeight())
    {
        do
        {
            pos.setY((int)waterY);
            if (!level->getBlock(pos)->isLiquidBlock())
            {
                const float floorY = waterY - step;
                const float upper = floorY - upperOffset;
                const float lower = floorY - lowerOffset;
                maxY = upper < upperBound ? upper : upperBound;
                waterY = floorY;
                minY = lower > lowerBound ? lower : lowerBound;
                const float top = (float)topY;
                if (top >= minY)
                {
                    minY = top + 0.5f;
                }
                if (maxY < minY)
                {
                    maxY = minY;
                }
                return;
            }
            waterY += step;
        }
        while (waterY < (float)level->getMaxBuildHeight());
    }

    const float upper = waterY - upperOffset;
    maxY = upper < height + yOffset ? upper : height + yOffset;
    const float lower = waterY - lowerOffset;
    minY = lower > height - yOffset ? lower : height - yOffset;
    const float top = (float)topY;
    if (minY <= top)
    {
        minY = top + 0.5f;
    }
    if (maxY < minY)
    {
        maxY = minY;
    }
}

Vec3* RandomSwimmingGoal::getPosition()
{
    Entity* entity = (Entity*)mMob;
    if (!mMob->getNavigation()->isDone() || mMob->getNavigation()->isRiverFollowing())
    {
        return NULL;
    }

    boost::shared_ptr<PathfinderMob> mob =
        boost::static_pointer_cast<PathfinderMob>(entity->shared_from_this());
    Vec3* target = NULL;
    int attempts = 0;
    float minY = 0.0f;
    float maxY = 0.0f;
    float waterY = 0.0f;
    int topY = 0;

    do
    {
        target = RandomPos::getPos(mob, mXz, mY, -1);
        ++attempts;
        if (target == NULL)
        {
            break;
        }

        BlockPos blockPos(target);
        MutableBlockPos pos(blockPos);
        getWaterHeights(pos, topY, waterY, minY, maxY);
        target->y = (double)entity->mRandom->nextFloat(minY, maxY);

        if (attempts >= 30)
        {
            break;
        }
    }
    while (entity->mLevel->getBlock(BlockPos(target))->isLiquidBlock());

    if (target == NULL)
    {
        return NULL;
    }

    Vec3* delta = target->subtract(entity->getPos(1.0f));
    const double horizontalDistance = std::sqrt(delta->x * delta->x + delta->z * delta->z);
    const double angle = Math::wrapDegrees(-std::atan2(delta->y, horizontalDistance));
    BlockPos destination((int)target->x, (int)std::ceil(target->y), (int)target->z);
    if (entity->mLevel->getBlock(destination)->isLiquidBlock()
        || (angle > 50.0 && angle < 130.0)
        || (angle < -50.0 && angle > -130.0))
    {
        return NULL;
    }
    return target;
}

bool RandomSwimmingGoal::canContinueToUse()
{
    Entity* entity = (Entity*)mMob;
    if (mMob->getNavigation()->isRiverFollowing() || !entity->isInWaterOrBubble())
    {
        return false;
    }
    return RandomStrollGoal::canContinueToUse();
}

bool RandomSwimmingGoal::canUse()
{
    Entity* entity = (Entity*)mMob;
    if (mMob->getNavigation()->isRiverFollowing() || !entity->isInWaterOrBubble())
    {
        return false;
    }
    return RandomStrollGoal::canUse();
}

std::wstring RandomSwimmingGoal::getName()
{
    return std::wstring(L"RandomSwimming");
}
