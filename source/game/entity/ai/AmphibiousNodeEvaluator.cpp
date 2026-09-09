#include "game/entity/ai/AmphibiousNodeEvaluator.hpp"

#include "game/core/Node.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"

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
    if (mMob->isInWater())
        return static_cast<double>(pos.y) + 0.5;
    return static_cast<double>(pos.y) - 0.3;
}

Node* AmphibiousNodeEvaluator::getAcceptedNode(int x, int y, int z, int verticalRange, double maxFallDistance)
{
    BlockPos position(x, y, z);
    if (inWaterDependentPosHeight(position) - maxFallDistance > 0.0)
        return NULL;

    Node* node = NodeEvaluator::getNode(x, y, z);
    const BlockPathTypes* type = WalkNodeEvaluator::getBlockPathType(mLevelSource, x, y, z, mMob,
                                                                      mEntityWidth, mEntityHeight, mEntityY,
                                                                      isCanOpenDoors(), isCanPassDoors());
    if (type == BlockPathTypes::BLOCKED)
        return NULL;
    if (type == BlockPathTypes::WATER || type == BlockPathTypes::WATER_BORDER)
        return node;
    if (type == BlockPathTypes::WALKABLE)
    {
        AABB* box = AABB::newTemp(static_cast<double>(x) - mMob->mBbWidth * 0.5,
                                   static_cast<double>(y) + 0.001,
                                   static_cast<double>(z) - mMob->mBbWidth * 0.5,
                                   static_cast<double>(x) + mMob->mBbWidth * 0.5,
                                   static_cast<double>(y) + mMob->mBbHeight,
                                   static_cast<double>(z) + mMob->mBbWidth * 0.5);
        if (!mMob->isFree(box))
            return NULL;
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
    if (node == NULL && verticalRange > 0 && type != BlockPathTypes::LAVA && type != BlockPathTypes::DAMAGE_FIRE)
        return getAcceptedNode(x, y + 1, z, verticalRange - 1, maxFallDistance);
    return node;
}

int AmphibiousNodeEvaluator::getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost)
{
    double height = inWaterDependentPosHeight(BlockPos(current->x, current->y, current->z));
    Node* candidates[10];
    candidates[0] = getAcceptedNode(current->x, current->y, current->z + 1, 1, height);
    candidates[1] = getAcceptedNode(current->x - 1, current->y, current->z, 1, height);
    candidates[2] = getAcceptedNode(current->x + 1, current->y, current->z, 1, height);
    candidates[3] = getAcceptedNode(current->x, current->y, current->z - 1, 1, height);
    candidates[4] = getAcceptedNode(current->x, current->y + 1, current->z, 0, height);
    candidates[5] = getAcceptedNode(current->x, current->y - 1, current->z, 1, height);
    bool north = candidates[3] == NULL || candidates[3]->GetType() != BlockPathTypes::BLOCKED;
    bool south = candidates[0] == NULL || candidates[0]->GetType() != BlockPathTypes::BLOCKED;
    bool west = candidates[1] == NULL || candidates[1]->GetType() != BlockPathTypes::BLOCKED;
    bool east = candidates[2] == NULL || candidates[2]->GetType() != BlockPathTypes::BLOCKED;
    candidates[6] = north && west ? getAcceptedNode(current->x - 1, current->y, current->z - 1, 1, height) : NULL;
    candidates[7] = north && east ? getAcceptedNode(current->x + 1, current->y, current->z - 1, 1, height) : NULL;
    candidates[8] = south && west ? getAcceptedNode(current->x - 1, current->y, current->z + 1, 1, height) : NULL;
    candidates[9] = south && east ? getAcceptedNode(current->x + 1, current->y, current->z + 1, 1, height) : NULL;
    int count = 0;
    int i;
    for (i = 0; i < 10; ++i)
        if (candidates[i] != NULL && !candidates[i]->closed && candidates[i]->distanceTo(target) < maxCost)
            neighbors->data[count++] = candidates[i];
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
                BlockPos pos(x + dx, y + dy, z + dz);
                const BlockPathTypes* type = getBlockPathType(levelSource, pos.x, pos.y, pos.z);
                pathTypeFlags |= 1 << (type->ordinal() & 31);
                if (dx == 0 && dy == 0 && dz == 0) result = type;
                mBlockPathTypeMap[pos.hashCode()] = 1 << (type->ordinal() & 31);
            }
    return result;
}

const BlockPathTypes* AmphibiousNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z)
{
    const BlockPathTypes* type = getBlockPathTypeRaw(levelSource, x, y, z);
    if (type != BlockPathTypes::BLOCKED)
        return checkNeighbourBlocks(levelSource, x, y, z, type);
    static const int offsets[4][3] = {{0, -1, 0}, {0, 1, 0}, {-1, 0, 0}, {1, 0, 0}};
    int i;
    for (i = 0; i < 4; ++i)
        if (getBlockPathTypeRaw(levelSource, x + offsets[i][0], y + offsets[i][1], z + offsets[i][2]) == BlockPathTypes::WATER)
            return BlockPathTypes::WATER_BORDER;
    return BlockPathTypes::BLOCKED;
}
