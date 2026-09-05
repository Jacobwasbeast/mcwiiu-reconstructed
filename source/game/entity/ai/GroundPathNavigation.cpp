#include "game/entity/ai/GroundPathNavigation.hpp"

#include <cmath>

#include "game/core/Path.hpp"
#include "game/core/PathFinder.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/MutableBlockPos.hpp"

GroundPathNavigation::GroundPathNavigation(Mob* mob, Level* level, bool createPathFinder)
    : PathNavigation(mob, level),
      mNodeEvaluator(mob),
      mAvoidSun(false)
{
    if (createPathFinder)
    {
        this->createPathFinder();
    }
}

GroundPathNavigation::~GroundPathNavigation()
{
}

NodeEvaluator* GroundPathNavigation::getNodeEvaluator()
{
    return &mNodeEvaluator;
}

void GroundPathNavigation::createPathFinder()
{
    mNodeEvaluator.setCanPassDoors(true);
    mPathFinder = new PathFinder(&mNodeEvaluator);
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
    if (!mMob->mOnGround && (!canFloat() || !isInLiquid()))
    {
        if (!mMob->isInWater())
        {
            if (mMob->GetType() != 0x5D540047ULL && !canSinkInWaterWalk())
            {
                return false;
            }
        }
    }
    return true;
}

int GroundPathNavigation::getSurfaceY()
{
    if (mMob->isInWater() && canFloat())
    {
        int y = static_cast<int>(mMob->mY);
        MutableBlockPos pos(static_cast<int>(std::floor(mMob->mX)), y,
                             static_cast<int>(std::floor(mMob->mZ)));
        int checks = 0;
        while (mNodeEvaluator.getBlockPathType(static_cast<Mob*>(mMob), pos) == BlockPathTypes::WATER ||
               mNodeEvaluator.getBlockPathType(static_cast<Mob*>(mMob), pos) == BlockPathTypes::WATER_BORDER)
        {
            ++y;
            pos.setY(y);
            ++checks;
            if (checks > 16)
            {
                return static_cast<int>(mMob->mY);
            }
        }
        return y;
    }
    return static_cast<int>(mMob->mY + 0.5);
}

Vec3* GroundPathNavigation::getTempMobPos()
{
    return Vec3::newTemp(mMob->mX, getSurfaceY(), mMob->mZ);
}

Path* GroundPathNavigation::createPath(const BlockPos& pos)
{
    BlockPos target(pos);
    const BlockPathTypes* type = mNodeEvaluator.getBlockPathType(static_cast<Mob*>(mMob), target);
    if (type == BlockPathTypes::WATER && !mMob->isInWater() && !canSinkInWaterWalk())
    {
        while (target.y > 0 && mNodeEvaluator.getBlockPathType(static_cast<Mob*>(mMob), target) == type)
        {
            target = target.below();
        }
        if (target.y > 0)
        {
            return PathNavigation::createPath(target.above());
        }
    }
    if (type == BlockPathTypes::LAVA && static_cast<Mob*>(mMob)->getPathfindingMalus(type) >= 0.0F)
    {
        while (target.y < mLevel->getMaxBuildHeight() &&
               static_cast<Mob*>(mMob)->getPathfindingMalus(
                   mNodeEvaluator.getBlockPathType(static_cast<Mob*>(mMob), target.above())) >= 0.0F)
        {
            target = target.above();
        }
        return PathNavigation::createPath(target);
    }
    if (type == BlockPathTypes::WATER && canSinkInWaterWalk())
    {
        while (target.y > 0 && mNodeEvaluator.getBlockPathType(static_cast<Mob*>(mMob), target) == type)
        {
            target = target.below();
        }
        if (target.y > 0)
        {
            return PathNavigation::createPath(target.above());
        }
    }
    return PathNavigation::createPath(target);
}

Path* GroundPathNavigation::createPath(boost::shared_ptr<Entity> entity)
{
    BlockPos pos(entity);
    return createPath(pos);
}

void GroundPathNavigation::trimPath()
{
    PathNavigation::trimPath();
    if (!mAvoidSun)
    {
        return;
    }
    BlockPos pos(mMob->mX, mMob->mY + 0.5, mMob->mZ);
    if (mLevel->canSeeSky(pos))
    {
        return;
    }
    MutableBlockPos pathPos;
    int removeCount = -1;
    int index = 0;
    while (index < mPath->getSize())
    {
        Node* node = mPath->get(index);
        pathPos.set(node->x, node->y, node->z);
        if (mLevel->canSeeSky(pathPos))
        {
            break;
        }
        ++removeCount;
        ++index;
    }
    if (index == mPath->getSize())
    {
        return;
    }
    mPath->setSize(index == 0 ? 0 : removeCount);
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
                if ((xStep * (ix + 0.5 - pos->x) + zStep * (iz + 0.5 - pos->z)) >= 0.52 &&
                    static_cast<Mob*>(mMob)->getPathfindingMalus(
                        mNodeEvaluator.getBlockPathType(static_cast<Mob*>(mMob), blockPos)) < 0.0F)
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
    if (!canWalkAbove(x - sizeX / 2, y - 1, z - sizeZ / 2, sizeX, sizeY, sizeZ,
                      pos, xStep, zStep))
    {
        return false;
    }
    return true;
}

bool GroundPathNavigation::canMoveDirectly(Vec3* start, Vec3* end, int width, int height, int depth)
{
    const double xDelta = end->x - start->x;
    const double zDelta = end->z - start->z;
    const double length = std::sqrt(xDelta * xDelta + zDelta * zDelta);
    if (length < 1.0)
    {
        return false;
    }
    const double xStep = xDelta / length;
    const double zStep = zDelta / length;
    int x = static_cast<int>(std::floor(start->x));
    int z = static_cast<int>(std::floor(start->z));
    int endX = static_cast<int>(std::floor(end->x));
    int endZ = static_cast<int>(std::floor(end->z));
    if (!canWalkOn(x, static_cast<int>(start->y), z, width + 2, height, depth + 2, start, xStep, zStep))
    {
        return false;
    }
    const int xDirection = xStep < 0.0 ? -1 : 1;
    const int zDirection = zStep < 0.0 ? -1 : 1;
    double xDistance = (x + (xStep >= 0.0 ? 1.0 : 0.0) - start->x) / xStep;
    double zDistance = (z + (zStep >= 0.0 ? 1.0 : 0.0) - start->z) / zStep;
    while (x != endX || z != endZ)
    {
        if (zDistance > xDistance)
        {
            x += xDirection;
            xDistance += 1.0 / std::fabs(xStep);
        }
        else
        {
            z += zDirection;
            zDistance += 1.0 / std::fabs(zStep);
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
