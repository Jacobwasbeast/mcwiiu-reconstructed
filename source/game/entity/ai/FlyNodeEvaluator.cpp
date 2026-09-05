#include "game/entity/ai/FlyNodeEvaluator.hpp"

#include <boost/unordered_set.hpp>

#include "game/core/Node.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"

static int addFlyNeighbor(FlyNodeEvaluator* evaluator, arrayWithLength<Node*>* neighbors,
                          Node* target, float maxCost, int count, int x, int y, int z)
{
    Node* node = evaluator->getNode(x, y, z);
    if (node != NULL && !node->closed && node->distanceTo(target) < maxCost)
        neighbors->data[count++] = node;
    return count;
}

FlyNodeEvaluator::FlyNodeEvaluator(Entity* entity)
    : WalkNodeEvaluator(entity)
{
}

void FlyNodeEvaluator::prepare(LevelSource* levelSource, Mob* mob)
{
    WalkNodeEvaluator::prepare(levelSource, mob);
}

void FlyNodeEvaluator::done()
{
    mMob->setPathfindingMalus(BlockPathTypes::WATER, mOldWaterCost);
    WalkNodeEvaluator::done();
}

const BlockPathTypes* FlyNodeEvaluator::getBlockPathType(Mob* mob, int x, int y, int z)
{
    return getBlockPathType(mLevelSource, x, y, z, mob, mEntityWidth, mEntityHeight, mEntityY,
                            isCanOpenDoors(), isCanPassDoors());
}

const BlockPathTypes* FlyNodeEvaluator::getBlockPathType(Mob* mob, const BlockPos& pos)
{
    return getBlockPathType(mLevelSource, pos.x, pos.y, pos.z, mob, mEntityWidth, mEntityHeight, mEntityY,
                            isCanOpenDoors(), isCanPassDoors());
}

Node* FlyNodeEvaluator::getStart()
{
    int x = static_cast<int>(mMob->mX);
    if (mMob->mX < static_cast<double>(x)) --x;
    int z = static_cast<int>(mMob->mZ);
    if (mMob->mZ < static_cast<double>(z)) --z;
    int y = static_cast<int>(mMob->mY + 0.5);
    if (mMob->mY + 0.5 < static_cast<double>(y)) --y;

    if (isCanFloat() && mMob->canFly())
    {
        y = static_cast<int>(mMob->mBoundingBox->mMinY);
        if (mMob->mBoundingBox->mMinY < static_cast<double>(y)) --y;
        while (getBlockPathType(mLevelSource, x, y, z) == BlockPathTypes::WATER)
            ++y;
        while (getBlockPathType(mLevelSource, x, y, z) == BlockPathTypes::WATER_BORDER)
            ++y;
    }

    const BlockPathTypes* startType = getBlockPathType(mMob, x, y, z);
    if (mMob->getPathfindingMalus(startType) >= 0.0)
        return NodeEvaluator::getNode(x, y, z);

    const int offsets[3] = { -1, 0, 1 };
    boost::unordered_set<BlockPos> positions;
    positions.insert(BlockPos(mMob->mX, y, mMob->mZ));
    positions.insert(BlockPos(mMob->mX, y, mMob->mBoundingBox->mMaxZ));
    positions.insert(BlockPos(mMob->mBoundingBox->mMaxX, y, mMob->mZ));
    positions.insert(BlockPos(mMob->mBoundingBox->mMaxX, y, mMob->mBoundingBox->mMaxZ));

    for (boost::unordered_set<BlockPos>::const_iterator it = positions.begin(); it != positions.end(); ++it)
    {
        const BlockPathTypes* type = getBlockPathType(mMob, *it);
        if (mMob->getPathfindingMalus(type) >= 0.0)
            return NodeEvaluator::getNode(it->x, it->y, it->z);
    }
    return NodeEvaluator::getNode(x, y, z);
}

Node* FlyNodeEvaluator::getGoal(double x, double y, double z)
{
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int iz = static_cast<int>(z);
    if (x < static_cast<double>(ix)) --ix;
    if (y < static_cast<double>(iy)) --iy;
    if (z < static_cast<double>(iz)) --iz;
    return NodeEvaluator::getNode(ix, iy, iz);
}

int FlyNodeEvaluator::getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost)
{
    int count = 0;
    const int x = current->x;
    const int y = current->y;
    const int z = current->z;
    int dx, dy, dz;
    for (dy = -1; dy <= 1; ++dy)
        for (dx = -1; dx <= 1; ++dx)
            for (dz = -1; dz <= 1; ++dz)
                if (dx != 0 || dy != 0 || dz != 0)
                    count = addFlyNeighbor(this, neighbors, target, maxCost, count, x + dx, y + dy, z + dz);
    return count;
}

Node* FlyNodeEvaluator::getNode(int x, int y, int z)
{
    const BlockPathTypes* type = getBlockPathType(mMob, x, y, z);
    double malus = mMob->getPathfindingMalus(type);
    if (malus < 0.0)
        return NULL;
    return NodeEvaluator::getNode(x, y, z);
}

const BlockPathTypes* FlyNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z,
                                                          Mob* mob, int sizeX, int sizeY, int sizeZ,
                                                          bool canOpenDoors, bool canPassDoors)
{
    int flags = 0;
    const BlockPathTypes* result = WalkNodeEvaluator::getBlockPathTypes(levelSource, x, y, z, sizeX, sizeY, sizeZ,
                                                                          canOpenDoors, canPassDoors, flags,
                                                                          BlockPathTypes::BLOCKED);
    if ((flags & (1 << (result->ordinal() & 31))) == 0)
        return result;
    for (int i = 0; i < 19; ++i)
    {
        const BlockPathTypes* type = BlockPathTypes::VALUES[i];
        if ((flags & (1 << (type->ordinal() & 31))) != 0)
        {
            double malus = mob->getPathfindingMalus(type);
            if (malus < 0.0)
                return type;
            if (malus >= mob->getPathfindingMalus(result))
                result = type;
        }
    }
    return result;
}

const BlockPathTypes* FlyNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z)
{
    const BlockPathTypes* result = WalkNodeEvaluator::getBlockPathTypeRaw(levelSource, x, y, z);
    if (result == BlockPathTypes::BLOCKED && y > 0)
    {
        BlockPos below(x, y - 1, z);
        const BlockPathTypes* belowType = WalkNodeEvaluator::getBlockPathTypeRaw(levelSource, x, y - 1, z);
        if (belowType == BlockPathTypes::OPEN || belowType == BlockPathTypes::WALKABLE || belowType == BlockPathTypes::WATER)
            result = belowType;
    }
    return WalkNodeEvaluator::checkNeighbourBlocks(levelSource, x, y, z, result);
}
