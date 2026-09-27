#include "game/entity/ai/AmphibiousNodeEvaluator.hpp"

#include <algorithm>

#include "game/core/EntityType.hpp"
#include "game/core/Node.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/Direction.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockPos.hpp"

AmphibiousNodeEvaluator::AmphibiousNodeEvaluator(Entity* entity)
    : WalkNodeEvaluator(entity)
{
}

void AmphibiousNodeEvaluator::prepare(LevelSource* levelSource, Mob* mob)
{
    WalkNodeEvaluator::prepare(levelSource, mob);
    mob->setPathfindingMalus(BlockPathTypes::WATER, 0.0f);
    mOldWalkableCost = mob->getPathfindingMalus(BlockPathTypes::WALKABLE);
    mob->setPathfindingMalus(BlockPathTypes::WALKABLE, 6.0f);
    mOldWaterBorderCost = mob->getPathfindingMalus(BlockPathTypes::WATER_BORDER);
    mob->setPathfindingMalus(BlockPathTypes::WATER_BORDER, 4.0f);
}

void AmphibiousNodeEvaluator::done()
{
    mMob->setPathfindingMalus(BlockPathTypes::WALKABLE, mOldWalkableCost);
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

Node* AmphibiousNodeEvaluator::getAcceptedNode(int x, int y, int z, int verticalRange,
                                               double maxFallDistance)
{
    Node* cachedNode = getNode(x, y, z);
    if (cachedNode->hasAcceptedNode)
        return cachedNode->acceptedNode;

    Node* node = NULL;
    BlockPos pos(x, y, z);
    if (inWaterDependentPosHeight(pos) - maxFallDistance > 1.125)
    {
        cachedNode->hasAcceptedNode = true;
        cachedNode->acceptedNode = NULL;
        return NULL;
    }

    const BlockPathTypes* type =
        WalkNodeEvaluator::getBlockPathType(mLevelSource, x, y, z, mMob, mEntityWidth, mEntityHeight,
                                            mEntityDepth, isCanOpenDoors(), isCanPassDoors());
    float malus = mMob->getPathfindingMalus(type);
    const double halfWidth = mMob->mBbWidth * 0.5;
    if (malus >= 0.0f)
    {
        node = getNode(x, y, z);
        node->type = type;
        node->costMalus = std::max(node->costMalus, malus);
    }

    if (type == BlockPathTypes::WATER || type == BlockPathTypes::WALKABLE)
    {
        if (y < mMob->mLevel->getSeaLevel() - 10 && node != NULL)
            node->costMalus += 1.0f;
        cachedNode->hasAcceptedNode = true;
        cachedNode->acceptedNode = node;
        return node;
    }

    if (node == NULL && verticalRange > 0 && type != BlockPathTypes::FENCE && type != BlockPathTypes::TRAPDOOR)
        node = getAcceptedNode(x, y + 1, z, verticalRange - 1, maxFallDistance);

    if (type == BlockPathTypes::OPEN)
    {
        AABB* box = AABB::newTemp(x - halfWidth + 0.5, y + 0.001, z - halfWidth + 0.5, x + halfWidth + 0.5,
                                  static_cast<float>(y) + mMob->mBbHeight, z + halfWidth + 0.5);
        if (mMob->mLevel->hasBlockCubes(box))
        {
            cachedNode->hasAcceptedNode = true;
            cachedNode->acceptedNode = NULL;
            return NULL;
        }

        const BlockPathTypes* below =
            WalkNodeEvaluator::getBlockPathType(mLevelSource, x, y - 1, z, mMob, mEntityWidth, mEntityHeight,
                                                mEntityDepth, isCanOpenDoors(), isCanPassDoors());
        if (below == BlockPathTypes::BLOCKED || below == BlockPathTypes::WATER)
        {
            node = getNode(x, y, z);
            cachedNode->hasAcceptedNode = true;
            cachedNode->acceptedNode = node;
            node->type = below == BlockPathTypes::BLOCKED ? BlockPathTypes::WALKABLE : BlockPathTypes::WATER;
            node->costMalus = std::max(node->costMalus, malus);
            return node;
        }

        int fallDistance = 0;
        while (y > 0 && type == BlockPathTypes::OPEN)
        {
            --y;
            if (fallDistance++ >= mMob->getMaxFallDistance())
            {
                cachedNode->hasAcceptedNode = true;
                cachedNode->acceptedNode = NULL;
                return NULL;
            }
            type = WalkNodeEvaluator::getBlockPathType(mLevelSource, x, y, z, mMob, mEntityWidth, mEntityHeight,
                                                       mEntityDepth, false, false);
            malus = mMob->getPathfindingMalus(type);
            if (malus < 0.0f)
            {
                cachedNode->hasAcceptedNode = true;
                cachedNode->acceptedNode = NULL;
                return NULL;
            }
            if (type != BlockPathTypes::OPEN)
            {
                node = getNode(x, y, z);
                node->type = type;
                node->costMalus = std::max(node->costMalus, malus);
            }
        }
    }

    cachedNode->hasAcceptedNode = true;
    cachedNode->acceptedNode = node;
    return node;
}

int AmphibiousNodeEvaluator::getNeighbors(arrayWithLength<Node*>* neighbors, Node* current,
                                          Node* target, float maxCost)
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
    candidates[6] = north && west
        ? getAcceptedNode(current->x - 1, current->y, current->z - 1, 1, height) : NULL;
    candidates[7] = north && east
        ? getAcceptedNode(current->x + 1, current->y, current->z - 1, 1, height) : NULL;
    candidates[8] = south && west
        ? getAcceptedNode(current->x - 1, current->y, current->z + 1, 1, height) : NULL;
    candidates[9] = south && east
        ? getAcceptedNode(current->x + 1, current->y, current->z + 1, 1, height) : NULL;
    int count = 0;
    int i;
    for (i = 0; i < 10; ++i)
        if (candidates[i] != NULL && !candidates[i]->closed &&
            candidates[i]->distanceTo(target) < maxCost)
            neighbors->data[count++] = candidates[i];
    return count;
}

const BlockPathTypes* AmphibiousNodeEvaluator::getBlockPathTypes(LevelSource* levelSource, int x,
                                                                 int y, int z, int sizeX, int sizeY,
                                                                 int sizeZ, bool canOpenDoors,
                                                                 bool canPassDoors,
                                                                 int& pathTypeFlags,
                                                                 const BlockPathTypes* pathType)
{
    BlockPos mobPos(mMob);
    for (int dx = 0; dx < sizeX; ++dx)
        for (int dy = 0; dy < sizeY; ++dy)
            for (int dz = 0; dz < sizeZ; ++dz)
            {
                int key = BlockPos(x + dx, y + dy, z + dz).hashCode();
                boost::unordered_map<int, int>::iterator cached = mBlockPathTypeMap.find(key);
                if (cached != mBlockPathTypeMap.end())
                {
                    pathTypeFlags |= cached->second;
                    continue;
                }

                const BlockPathTypes* type = getBlockPathType(levelSource, x + dx, y + dy, z + dz);
                if (type == BlockPathTypes::RAIL
                    && !levelSource->getBlockState(mobPos)->getBlock()->instanceof(EBLOCK_TYPE_RAIL)
                    && !levelSource->getBlockState(mobPos.below())->getBlock()->instanceof(EBLOCK_TYPE_RAIL))
                    type = BlockPathTypes::FENCE;

                if (type == BlockPathTypes::DOOR_OPEN || type == BlockPathTypes::DOOR_WOOD_CLOSED
                    || type == BlockPathTypes::DOOR_IRON_CLOSED)
                {
                    if (mMob->GetType() == EntityType::TURTLE)
                        type = BlockPathTypes::BLOCKED;
                    else if (type == BlockPathTypes::DOOR_WOOD_CLOSED && canOpenDoors && canPassDoors)
                        type = BlockPathTypes::WALKABLE;
                    else if (type == BlockPathTypes::DOOR_OPEN && !canPassDoors)
                        type = BlockPathTypes::BLOCKED;
                }
                if (dx == 0 && dy == 0 && dz == 0)
                    pathType = type;

                pathTypeFlags |= 1 << type->ordinal();
                mBlockPathTypeMap[key] = 1 << type->ordinal();
            }
    return pathType;
}

const BlockPathTypes* AmphibiousNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z)
{
    const BlockPathTypes* type = getBlockPathTypeRaw(levelSource, x, y, z);
    if (type == BlockPathTypes::WATER)
    {
        for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
             ++it)
        {
            const Direction* direction = *it;
            if (getBlockPathTypeRaw(levelSource, x + direction->getStepX(), y + direction->getStepY(),
                                    z + direction->getStepZ()) == BlockPathTypes::BLOCKED)
                return BlockPathTypes::WATER_BORDER;
        }
        return BlockPathTypes::WATER;
    }

    if (type == BlockPathTypes::OPEN && y >= 1)
    {
        Block* blockBelow = levelSource->getBlockState(BlockPos(x, y - 1, z))->getBlock();
        const BlockPathTypes* typeBelow = getBlockPathTypeRaw(levelSource, x, y - 1, z);
        if (typeBelow == BlockPathTypes::WALKABLE || typeBelow == BlockPathTypes::OPEN
            || typeBelow == BlockPathTypes::LAVA)
            type = BlockPathTypes::OPEN;
        else
            type = BlockPathTypes::WALKABLE;
        if (typeBelow == BlockPathTypes::DAMAGE_FIRE || blockBelow == Blocks::magma)
            type = BlockPathTypes::DAMAGE_FIRE;
        if (typeBelow == BlockPathTypes::DAMAGE_CACTUS)
            type = BlockPathTypes::DAMAGE_CACTUS;
    }
    return checkNeighbourBlocks(levelSource, x, y, z, type);
}
