#include "game/entity/ai/AmphibiousNodeEvaluator.hpp"
#include "game/core/Node.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

AmphibiousNodeEvaluator::AmphibiousNodeEvaluator(Entity* entity) : WalkNodeEvaluator(entity) {}

void AmphibiousNodeEvaluator::prepare(LevelSource* levelSource, Mob* mob)
{
    WalkNodeEvaluator::prepare(levelSource, mob);
    mOldWaterCost = mob->getPathfindingMalus(BlockPathTypes::WATER);
    mob->setPathfindingMalus(BlockPathTypes::WATER, 0.0f);
    mOldWaterBorderCost = mob->getPathfindingMalus(BlockPathTypes::WATER_BORDER);
    mob->setPathfindingMalus(BlockPathTypes::WATER_BORDER, 6.0f);
}

void AmphibiousNodeEvaluator::done()
{
    mMob->setPathfindingMalus(BlockPathTypes::WATER, mOldWaterCost);
    mMob->setPathfindingMalus(BlockPathTypes::WATER_BORDER, mOldWaterBorderCost);
    WalkNodeEvaluator::done();
}

Node* AmphibiousNodeEvaluator::getStart()
{
    int x = static_cast<int>(mMob->mX);
    int y = static_cast<int>(mMob->mY + 0.5);
    int z = static_cast<int>(mMob->mZ);
    if (mMob->mX < static_cast<double>(x)) --x;
    if (mMob->mY + 0.5 < static_cast<double>(y)) --y;
    if (mMob->mZ < static_cast<double>(z)) --z;
    return NodeEvaluator::getNode(x, y, z);
}

Node* AmphibiousNodeEvaluator::getGoal(double x, double y, double z)
{
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y + 0.5);
    int iz = static_cast<int>(z);
    if (x < static_cast<double>(ix)) --ix;
    if (y + 0.5 < static_cast<double>(iy)) --iy;
    if (z < static_cast<double>(iz)) --iz;
    return NodeEvaluator::getNode(ix, iy, iz);
}

double AmphibiousNodeEvaluator::inWaterDependentPosHeight(const BlockPos& pos)
{
    if (!mMob->isInWater())
        return static_cast<double>(pos.y) - 0.3;
    return static_cast<double>(pos.y) + 0.5;
}

Node* AmphibiousNodeEvaluator::getAcceptedNode(int x, int y, int z, int verticalRange, double maxFallDistance)
{
    Node* node = NodeEvaluator::getNode(x, y, z);
    const BlockPathTypes* type = getBlockPathType(mLevelSource, x, y, z);
    if (type == BlockPathTypes::BLOCKED)
        return 0;
    if (type == BlockPathTypes::WATER || type == BlockPathTypes::WATER_BORDER)
        return node;
    if (type == BlockPathTypes::WALKABLE)
    {
        AABB* box = AABB::newTemp(static_cast<double>(x) - mMob->mBbWidth * 0.5, static_cast<double>(y) + 0.001, static_cast<double>(z) - mMob->mBbWidth * 0.5, static_cast<double>(x) + mMob->mBbWidth * 0.5, static_cast<double>(y) + mMob->mBbHeight, static_cast<double>(z) + mMob->mBbWidth * 0.5);
        if (mMob->mLevel->hasBlockCubes(box))
            return 0;
        const BlockPathTypes* below = getBlockPathType(mLevelSource, x, y - 1, z);
        if (below == BlockPathTypes::OPEN || below == BlockPathTypes::WALKABLE || below == BlockPathTypes::WATER)
            return node;
        int depth = 0;
        while (y > 0 && depth < mMob->getMaxFallDistance() && getBlockPathType(mLevelSource, x, y - 1, z) == BlockPathTypes::WALKABLE)
        {
            --y;
            ++depth;
        }
    }
    if (node == 0 && verticalRange > 0 && type != BlockPathTypes::LAVA && type != BlockPathTypes::DAMAGE_FIRE)
        return getAcceptedNode(x, y + 1, z, verticalRange - 1, maxFallDistance);
    return node;
}

int AmphibiousNodeEvaluator::getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost)
{
    const double height = inWaterDependentPosHeight(BlockPos(current->x, current->y, current->z));
    Node* nodes[6];
    nodes[0] = getAcceptedNode(current->x, current->y, current->z + 1, 1, height);
    nodes[1] = getAcceptedNode(current->x - 1, current->y, current->z, 1, height);
    nodes[2] = getAcceptedNode(current->x + 1, current->y, current->z, 1, height);
    nodes[3] = getAcceptedNode(current->x, current->y, current->z - 1, 1, height);
    nodes[4] = getAcceptedNode(current->x, current->y + 1, current->z, 0, height);
    nodes[5] = getAcceptedNode(current->x, current->y - 1, current->z, 1, height);
    int count = 0;
    int i;
    for (i = 0; i < 6; ++i)
        if (nodes[i] != 0 && !nodes[i]->closed && nodes[i]->distanceTo(target) < maxCost)
            neighbors->data[count++] = nodes[i];
    return count;
}

const BlockPathTypes* AmphibiousNodeEvaluator::getBlockPathTypes(LevelSource* levelSource, int x, int y, int z, int sizeX, int sizeY, int sizeZ, bool canOpenDoors, bool canPassDoors, int& pathTypeFlags, const BlockPathTypes* preferred)
{
    const BlockPathTypes* result = preferred;
    pathTypeFlags = 0;
    int dx, dy, dz;
    for (dx = 0; dx < sizeX; ++dx)
        for (dy = 0; dy < sizeY; ++dy)
            for (dz = 0; dz < sizeZ; ++dz)
            {
                const BlockPathTypes* type = getBlockPathType(levelSource, x + dx, y + dy, z + dz);
                pathTypeFlags |= 1 << (type->ordinal() & 31);
                if (dx == 0 && dy == 0 && dz == 0)
                    result = type;
                mBlockPathTypeMap[BlockPos(x + dx, y + dy, z + dz).hashCode()] = 1 << (type->ordinal() & 31);
            }
    return result;
}

const BlockPathTypes* AmphibiousNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z)
{
    const BlockPathTypes* type = getBlockPathTypeRaw(levelSource, x, y, z);
    if (type != BlockPathTypes::BLOCKED)
        return checkNeighbourBlocks(levelSource, x, y, z, type);
    const int offsets[4][3] = {{0, -1, 0}, {0, 1, 0}, {-1, 0, 0}, {1, 0, 0}};
    int i;
    for (i = 0; i < 4; ++i)
        if (getBlockPathTypeRaw(levelSource, x + offsets[i][0], y + offsets[i][1], z + offsets[i][2]) == BlockPathTypes::WATER)
            return BlockPathTypes::WATER_BORDER;
    return BlockPathTypes::BLOCKED;
}
