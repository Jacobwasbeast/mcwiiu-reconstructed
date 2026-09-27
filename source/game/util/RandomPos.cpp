#include "game/util/RandomPos.hpp"

#include <math.h>

#include "game/core/Mth.hpp"
#include "game/util/Vec3.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

#include "game/world/Level.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/entity/ai/PathNavigation.hpp"

static const float PI = 3.1415927f;
static const float HALF_PI = 1.5707964f;
static const float SQRT_OF_TWO = Mth::sqrt(2.0);

BlockPos* RandomPos::getRandomDelta(Random* random, int xz, int y, const Vec3* direction, double maxAngleDelta)
{
    if (direction == NULL || maxAngleDelta >= PI)
    {
        int dx = random->nextInt(xz * 2 + 1) - xz;
        int dy = random->nextInt(y * 2 + 1) - y;
        int dz = random->nextInt(xz * 2 + 1) - xz;

        return new BlockPos(dx, dy, dz);
    }

    float angle = Mth::atan2((int)direction->z, (int)direction->x) - HALF_PI;
    double target = (double)angle + (double)(2.0f * random->nextFloat() - 1.0f) * maxAngleDelta;
    double distance = sqrt(random->nextDouble()) * (double)SQRT_OF_TWO * (double)xz;
    double dx = -distance * sin(target);
    double dz = distance * cos(target);

    if (fabs(dx) > (double)xz || fabs(dz) > (double)xz)
    {
        return NULL;
    }

    int dy = random->nextInt(y * 2 + 1) - y;

    return new BlockPos(dx, (double)dy, dz);
}

BlockPos RandomPos::moveAboveSolid(const BlockPos& pos, const boost::shared_ptr<PathfinderMob>& mob)
{
    Level* level = ((Entity*)mob.get())->mLevel;

    if (!level->getBlockState(pos)->getMaterial()->isSolid())
    {
        return pos;
    }

    BlockPos above = pos.above();

    while (above.y < level->getHeight() && level->getBlockState(above)->getMaterial()->isSolid())
    {
        above = above.above();
    }

    return above;
}

bool RandomPos::isWaterDestination(const BlockPos& pos, const boost::shared_ptr<PathfinderMob>& mob)
{
    Level* level = ((Entity*)mob.get())->mLevel;
    return level->getBlockState(pos)->getMaterial()->isUnderWater();
}

Vec3* RandomPos::generateRandomPos(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, const Vec3* direction, bool allowWater, double maxAngleDelta, int wanderingQuadrant)
{
    PathNavigation* navigation = mob->getNavigation();
    Random* random = mob->getRandom();
    bool checkRestriction;

    Entity* entity = (Entity*)mob.get();

    if (mob->hasRestriction())
    {
        BlockPos restrictCenter = mob->getRestrictCenter();
        double distanceSqr = restrictCenter.distSqr((double)Mth::floor(entity->mX), (double)Mth::floor(entity->mY),
                                                    (double)Mth::floor(entity->mZ)) + 4.0;
        float range = mob->getRestrictRadius() + (float)xz;

        checkRestriction = distanceSqr < (double)range * (double)range;
    }
    else
    {
        checkRestriction = false;
    }

    bool found = false;
    float bestValue = -99999.0f;
    int bestX = 0;
    int bestY = 0;
    int bestZ = 0;
    MutableBlockPos pos(0, 0, 0);

    for (int attempt = 0; attempt < 10; ++attempt)
    {
        BlockPos* delta = getRandomDelta(random, xz, y, direction, maxAngleDelta);

        if (delta == NULL)
        {
            continue;
        }

        int dx = delta->x;
        int dy = delta->y;
        int dz = delta->z;

        delete delta;

        if (wanderingQuadrant == -1)
        {
            dx = random->nextInt(xz * 2 + 1) - xz;
            dz = random->nextInt(xz * 2 + 1) - xz;
        }

        if (direction != NULL && (double)dx * direction->x + (double)dz * direction->z < 0.0)
        {
            continue;
        }

        if (mob->hasRestriction() && xz > 1)
        {
            BlockPos restrictCenter = mob->getRestrictCenter();

            if (entity->mX > (double)restrictCenter.x)
            {
                dx -= random->nextInt(xz / 2);
            }
            else
            {
                dx += random->nextInt(xz / 2);
            }

            if (entity->mZ > (double)restrictCenter.z)
            {
                dz -= random->nextInt(xz / 2);
            }
            else
            {
                dz += random->nextInt(xz / 2);
            }
        }

        pos.set((int)((double)dx + entity->mX), (int)((double)dy + entity->mY), (int)((double)dz + entity->mZ));

        if (checkRestriction && !mob->isWithinRestriction(pos))
        {
            continue;
        }

        if (!navigation->isStableDestination(pos))
        {
            continue;
        }

        if (!allowWater)
        {
            pos.set(moveAboveSolid(pos, mob));

            if (isWaterDestination(pos, mob))
            {
                continue;
            }
        }

        float value = mob->getWalkTargetValue(pos);

        if (value > bestValue)
        {
            bestValue = value;
            bestX = dx;
            bestY = dy;
            bestZ = dz;
            found = true;
        }
    }

    if (found)
    {
        return Vec3::newTemp((double)bestX + entity->mX, (double)bestY + entity->mY, (double)bestZ + entity->mZ);
    }

    return NULL;
}

Vec3* RandomPos::generateRandomPos(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, const Vec3* direction, int wanderingQuadrant)
{
    return generateRandomPos(mob, xz, y, direction, true, (double)HALF_PI, -1);
}

Vec3* RandomPos::getPos(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, int wanderingQuadrant)
{
    return generateRandomPos(mob, xz, y, NULL, wanderingQuadrant);
}

Vec3* RandomPos::getLandPos(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, int wanderingQuadrant)
{
    return generateRandomPos(mob, xz, y, NULL, false, 0.0, wanderingQuadrant);
}

Vec3* RandomPos::getPosTowards(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, Vec3* target)
{
    Entity* entity = (Entity*)mob.get();
    return generateRandomPos(mob, xz, y, target->subtract(entity->mX, entity->mY, entity->mZ), -1);
}

Vec3* RandomPos::getPosTowards(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, Vec3* target, double maxAngleDelta)
{
    Entity* entity = (Entity*)mob.get();
    return generateRandomPos(mob, xz, y, target->subtract(entity->mX, entity->mY, entity->mZ), true, maxAngleDelta, -1);
}

Vec3* RandomPos::getPosAvoid(const boost::shared_ptr<PathfinderMob>& mob, int xz, int y, Vec3* avoid)
{
    Entity* entity = (Entity*)mob.get();
    return generateRandomPos(mob, xz, y, Vec3::newTemp(entity->mX, entity->mY, entity->mZ)->subtract(avoid), -1);
}
