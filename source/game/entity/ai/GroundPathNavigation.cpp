#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"

GroundPathNavigation::GroundPathNavigation(Mob* mob, Level* level, bool createPathFinder)
    : PathNavigation(mob, level),
      mNodeEvaluator(mob)
{
    if (createPathFinder)
    {
        mPathFinder = this->createPathFinder();
    }
    mAvoidSun = false;
}

NodeEvaluator* GroundPathNavigation::getNodeEvaluator()
{
    return &mNodeEvaluator;
}

PathFinder* GroundPathNavigation::createPathFinder()
{
    mNodeEvaluator.setCanPassDoors(true);
    return new PathFinder(&mNodeEvaluator);
}

bool GroundPathNavigation::canFloat()
{
    return mNodeEvaluator.isCanFloat();
}

bool GroundPathNavigation::canSinkInWaterWalk()
{
    return mNodeEvaluator.isCanSinkInWaterWalk();
}

bool GroundPathNavigation::canUpdatePath()
{
    return mMob->mOnGround || (canFloat() && isInLiquid()) || mMob->isPassenger()
        || mMob->GetType() == EntityType::WITHER_BOSS || canSinkInWaterWalk();
}

int GroundPathNavigation::getSurfaceY()
{
    if (mMob->isInWater() && canFloat())
    {
        int y = static_cast<int>(mMob->mBoundingBox->mMinY);
        MutableBlockPos pos(static_cast<int>(std::floor(mMob->mX)), y,
                             static_cast<int>(std::floor(mMob->mZ)));
        int checks = 0;
        Block* block = mLevel->getBlock(pos);
        while (block == Blocks::flowing_water || block == Blocks::water)
        {
            ++y;
            pos.setY(y);
            block = mLevel->getBlock(pos);
            ++checks;
            if (checks > 16)
            {
                return static_cast<int>(mMob->mBoundingBox->mMinY);
            }
        }
        return y;
    }
    return static_cast<int>(mMob->mBoundingBox->mMinY + 0.5);
}

Vec3* GroundPathNavigation::getTempMobPos()
{
    return Vec3::newTemp(mMob->mX, getSurfaceY(), mMob->mZ);
}

Path* GroundPathNavigation::createPath(const BlockPos& pos)
{
    BlockPos target(pos);
    if (mLevel->getBlockState(target)->getMaterial() == Material::AIR && !mMob->canFly())
    {
        BlockPos column = target.below();
        while (column.y > 0 && mLevel->getBlockState(column)->getMaterial() == Material::AIR)
        {
            column = column.below();
        }
        if (column.y > 0)
        {
            return PathNavigation::createPath(column.above());
        }
        while (column.y < mLevel->getMaxBuildHeight() &&
               mLevel->getBlockState(column)->getMaterial() == Material::AIR)
        {
            column = column.above();
        }
        target = column;
    }
    if (mLevel->getBlockState(target)->getMaterial()->isSolid())
    {
        BlockPos column = target.above();
        while (column.y < mLevel->getMaxBuildHeight() &&
               mLevel->getBlockState(column)->getMaterial()->isSolid())
        {
            column = column.above();
        }
        return PathNavigation::createPath(column);
    }
    if (mLevel->getBlockState(target)->getMaterial() == Material::WATER && canSinkInWaterWalk())
    {
        BlockPos column = target.below();
        while (column.y > 0 && mLevel->getBlockState(column)->getMaterial() == Material::WATER)
        {
            column = column.below();
        }
        if (column.y > 0)
        {
            return PathNavigation::createPath(column.above());
        }
        while (column.y < mLevel->getMaxBuildHeight() &&
               mLevel->getBlockState(column)->getMaterial() == Material::WATER)
        {
            column = column.above();
        }
        target = column;
    }
    return PathNavigation::createPath(target);
}

Path* GroundPathNavigation::createPath(boost::shared_ptr<Entity> entity)
{
    return createPath(BlockPos(entity));
}

void GroundPathNavigation::trimPath()
{
    PathNavigation::trimPath();
    if (!mAvoidSun)
    {
        return;
    }
    if (mLevel->canSeeSky(BlockPos(mMob->mX, mMob->mBoundingBox->mMinY + 0.5, mMob->mZ)))
    {
        return;
    }
    MutableBlockPos pathPos;
    int removeCount = -1;
    for (int index = 0; index < mPath->getSize(); ++index)
    {
        Node* node = mPath->get(index);
        pathPos.set(node->x, node->y, node->z);
        if (mLevel->canSeeSky(pathPos))
        {
            if (index == 0)
            {
                mPath->setSize(0);
            }
            else
            {
                mPath->setSize(removeCount);
            }
            break;
        }
        ++removeCount;
    }
}

bool GroundPathNavigation::canWalkAbove(int x, int y, int z, int sizeX, int sizeY, int sizeZ,
                                        Vec3* pos, double xStep, double zStep)
{
    MutableBlockPos blockPos;
    for (int ix = x; ix < x + sizeX; ++ix)
    {
        blockPos.setX(ix);
        for (int iy = y; iy < y + sizeY; ++iy)
        {
            blockPos.setY(iy);
            for (int iz = z; iz < z + sizeZ; ++iz)
            {
                blockPos.setZ(iz);
                if ((xStep * (ix + 0.5 - pos->x) + zStep * (iz + 0.5 - pos->z)) >= 0.0 &&
                    !mLevel->getBlockState(mMob->mTickCache, blockPos)->getBlock()->isPathfindable(mLevel, blockPos))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool GroundPathNavigation::canWalkOn(int x, int y, int z, int sizeX, int sizeY, int sizeZ,
                                     Vec3* pos, double xStep, double zStep)
{
    int minX = x - sizeX / 2;
    int minZ = z - sizeZ / 2;
    if (!canWalkAbove(minX, y, minZ, sizeX, sizeY, sizeZ, pos, xStep, zStep))
    {
        return false;
    }
    if (!static_cast<Mob*>(mMob)->canFly())
    {
        MutableBlockPos blockPos(0, y - 1, 0);
        for (int ix = minX; ix < minX + sizeX; ++ix)
        {
            blockPos.setX(ix);
            for (int iz = minZ; iz < minZ + sizeZ; ++iz)
            {
                blockPos.setZ(iz);
                if (xStep * (ix + 0.5 - pos->x) + zStep * (iz + 0.5 - pos->z) >= 0.0)
                {
                    const BlockPathTypes* type = mNodeEvaluator.getBlockPathType(
                        mLevel, ix, y - 1, iz, static_cast<Mob*>(mMob), sizeX, sizeY, sizeZ, true, true);
                    if ((type == BlockPathTypes::WATER && !canSinkInWaterWalk()) || type == BlockPathTypes::LAVA
                        || type == BlockPathTypes::OPEN)
                    {
                        return false;
                    }
                    type = mNodeEvaluator.getBlockPathType(
                        mLevel, ix, y, iz, static_cast<Mob*>(mMob), sizeX, sizeY, sizeZ, true, true);
                    float malus = static_cast<Mob*>(mMob)->getPathfindingMalus(type);
                    if (malus < 0.0F || malus >= 8.0F || type == BlockPathTypes::DAMAGE_FIRE
                        || type == BlockPathTypes::DANGER_FIRE || type == BlockPathTypes::DAMAGE_OTHER)
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

bool GroundPathNavigation::canMoveDirectly(Vec3* start, Vec3* end, int width, int height, int depth)
{
    int x = static_cast<int>(std::floor(start->x));
    int z = static_cast<int>(std::floor(start->z));
    double xDelta = end->x - start->x;
    double zDelta = end->z - start->z;
    double lengthSquared = xDelta * xDelta + zDelta * zDelta;
    if (lengthSquared < 1.0E-8)
    {
        return false;
    }
    double length = std::sqrt(lengthSquared);
    double xStep = xDelta * (1.0 / length);
    double zStep = zDelta * (1.0 / length);
    if (!canWalkOn(x, static_cast<int>(start->y), z, width + 2, height, depth + 2, start, xStep, zStep))
    {
        return false;
    }
    double xDistance = x - start->x;
    double zDistance = z - start->z;
    if (xStep >= 0.0)
    {
        xDistance += 1.0;
    }
    if (zStep >= 0.0)
    {
        zDistance += 1.0;
    }
    xDistance /= xStep;
    zDistance /= zStep;
    int xDirection = xStep < 0.0 ? -1 : 1;
    int zDirection = zStep < 0.0 ? -1 : 1;
    int xRemaining = static_cast<int>(std::floor(end->x)) - x;
    int zRemaining = static_cast<int>(std::floor(end->z)) - z;
    while (xRemaining * xDirection > 0 || zRemaining * zDirection > 0)
    {
        if (xDistance < zDistance)
        {
            x += xDirection;
            xDistance += 1.0 / std::fabs(xStep);
            xRemaining -= xDirection;
        }
        else
        {
            z += zDirection;
            zDistance += 1.0 / std::fabs(zStep);
            zRemaining -= zDirection;
        }
        if (!canWalkOn(x, static_cast<int>(start->y), z, width, height, depth, start, xStep, zStep))
        {
            return false;
        }
    }
    return true;
}

void GroundPathNavigation::setCanOpenDoors(bool value)
{
    mNodeEvaluator.setCanOpenDoors(value);
}

void GroundPathNavigation::setCanPassDoors(bool value)
{
    mNodeEvaluator.setCanPassDoors(value);
}

bool GroundPathNavigation::canOpenDoors()
{
    return mNodeEvaluator.isCanPassDoors();
}

void GroundPathNavigation::setCanFloat(bool value)
{
    mNodeEvaluator.setCanFloat(value);
}

void GroundPathNavigation::setAvoidSun(bool value)
{
    mAvoidSun = value;
}

bool GroundPathNavigation::shouldAvoidSun() const
{
    return mAvoidSun;
}

void GroundPathNavigation::setSinkInWaterWalk(bool value)
{
    mNodeEvaluator.setSinkInWaterWalk(value);
}
