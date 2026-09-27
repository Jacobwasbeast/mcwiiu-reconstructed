#include "game/entity/ai/WalkNodeEvaluator.hpp"

#include <algorithm>

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FenceGateBlock.hpp"
#include "game/core/Mth.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BaseRailBlock.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/world/Level.hpp"
#include "game/core/Node.hpp"

WalkNodeEvaluator::WalkNodeEvaluator(Entity* entity)
    : NodeEvaluator(), mOldWaterCost(1.65f), mRailAtStart(false), mRailBelowStart(false), mEntity(entity)
{
}

void WalkNodeEvaluator::prepare(LevelSource* levelSource, Mob* mob)
{
    if (mEntity == NULL)
    {
        mEntity = mob;
    }

    NodeEvaluator::prepare(levelSource, mob);
    mOldWaterCost = mob->getPathfindingMalus(BlockPathTypes::WATER);
    BlockPos pos(mob);
    mRailAtStart = dynamic_cast<BaseRailBlock*>(levelSource->getBlock(pos)) != NULL;
    mRailBelowStart = dynamic_cast<BaseRailBlock*>(levelSource->getBlock(pos.below())) != NULL;
}

void WalkNodeEvaluator::done()
{
    mMob->setPathfindingMalus(BlockPathTypes::WATER, mOldWaterCost);
    NodeEvaluator::done();
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathType(Mob* mob, int x, int y, int z)
{
    return getBlockPathType(mLevelSource, x, y, z, mob, mEntityWidth, mEntityHeight, mEntityDepth,
                            isCanOpenDoors(), isCanPassDoors());
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathType(Mob* mob, const BlockPos& pos)
{
    return getBlockPathType(mob, pos.x, pos.y, pos.z);
}

Node* WalkNodeEvaluator::getStart()
{
    int y;
    if (isCanFloat() && mMob->isInWater())
    {
        const int x = Mth::floor(mMob->mX);
        const int z = Mth::floor(mMob->mZ);
        y = static_cast<int>(mMob->mY);
        int blockId = mLevelSource->getBlockId(mEntity->mTickCache, x, y, z);
        while (blockId == BlockId::FLOWING_WATER || blockId == BlockId::WATER)
            blockId = mLevelSource->getBlockId(mEntity->mTickCache, x, ++y, z);
    }
    else if (mMob->mOnGround)
    {
        y = Mth::floor(mMob->mY + 0.5);
    }
    else
    {
        MutableBlockPos pos(Mth::floor(mMob->mX), Mth::floor(mMob->mY), Mth::floor(mMob->mZ));
        while ((getBlockState(mEntity->mTickCache, mLevelSource, pos)->getMaterial() == Material::AIR
                || getBlockState(mEntity->mTickCache, mLevelSource, pos)->getBlock()->isPathfindable(mLevelSource, pos))
               && pos.y > 0)
            pos.setY(pos.y - 1);
        y = pos.above().y;
    }

    BlockPos mobPos(mMob);
    const BlockPathTypes* type = getBlockPathType(mMob, mobPos.x, y, mobPos.z);
    if (mMob->getPathfindingMalus(type) < 0.0f)
    {
        const AABB* box = mMob->mBoundingBox;
        boost::unordered_set<BlockPos> corners(11);
        corners.insert(BlockPos(box->mMinX, static_cast<double>(y), box->mMinZ));
        corners.insert(BlockPos(box->mMinX, static_cast<double>(y), box->mMaxZ));
        corners.insert(BlockPos(box->mMaxX, static_cast<double>(y), box->mMinZ));
        corners.insert(BlockPos(box->mMaxX, static_cast<double>(y), box->mMaxZ));
        for (boost::unordered_set<BlockPos>::const_iterator it = corners.begin(); it != corners.end(); ++it)
        {
            if (mMob->getPathfindingMalus(getBlockPathType(mMob, *it)) >= 0.0f)
                return getNode(it->x, it->y, it->z);
        }
    }
    return NodeEvaluator::getNode(mobPos.x, y, mobPos.z);
}

Node* WalkNodeEvaluator::getGoal(double x, double y, double z)
{
    int ix = static_cast<int>(x);
    int iy = static_cast<int>(y);
    int iz = static_cast<int>(z);
    if (x < static_cast<double>(ix)) --ix;
    if (y < static_cast<double>(iy)) --iy;
    if (z < static_cast<double>(iz)) --iz;
    return NodeEvaluator::getNode(ix, iy, iz);
}

int WalkNodeEvaluator::getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost)
{
    int count = 0;
    int verticalRange = 0;
    const BlockPathTypes* typeAbove = getBlockPathType(mMob, current->x, current->y + 1, current->z);
    if (mMob->getPathfindingMalus(typeAbove) >= 0.0f)
        verticalRange = Mth::floor(std::max(1.0f, mMob->mMaxUpStep));

    BlockPos posBelow = BlockPos(current->x, current->y, current->z).below();
    const double floorY = static_cast<double>(current->y)
                          - (1.0 - getBlockState(mEntity->mTickCache, mLevelSource, posBelow)
                                       ->getShape(mLevelSource, posBelow)->mMaxY);

    Node* south = getLandNode(current->x, current->y, current->z + 1, verticalRange, floorY, Direction::SOUTH);
    Node* west = getLandNode(current->x - 1, current->y, current->z, verticalRange, floorY, Direction::WEST);
    Node* east = getLandNode(current->x + 1, current->y, current->z, verticalRange, floorY, Direction::EAST);
    Node* north = getLandNode(current->x, current->y, current->z - 1, verticalRange, floorY, Direction::NORTH);

    if (south != NULL && !south->closed && south->distanceTo(target) < maxCost)
        neighbors->data[count++] = south;
    if (north != NULL && !north->closed && north->distanceTo(target) < maxCost)
        neighbors->data[count++] = north;
    if (west != NULL && !west->closed && west->distanceTo(target) < maxCost)
        neighbors->data[count++] = west;
    if (east != NULL && !east->closed && east->distanceTo(target) < maxCost)
        neighbors->data[count++] = east;

    const bool northOpen = north == NULL || north->type == BlockPathTypes::OPEN || north->costMalus != 0.0f;
    const bool southOpen = south == NULL || south->type == BlockPathTypes::OPEN || south->costMalus != 0.0f;
    const bool eastOpen = east == NULL || east->type == BlockPathTypes::OPEN || east->costMalus != 0.0f;
    const bool westOpen = west == NULL || west->type == BlockPathTypes::OPEN || west->costMalus != 0.0f;

    if (northOpen)
    {
        if (westOpen)
        {
            Node* node = getLandNode(current->x - 1, current->y, current->z - 1, verticalRange, floorY, Direction::NORTH);
            if (node != NULL && !node->closed && node->distanceTo(target) < maxCost)
                neighbors->data[count++] = node;
        }
        if (eastOpen)
        {
            Node* node = getLandNode(current->x + 1, current->y, current->z - 1, verticalRange, floorY, Direction::NORTH);
            if (node != NULL && !node->closed && node->distanceTo(target) < maxCost)
                neighbors->data[count++] = node;
        }
    }
    if (southOpen)
    {
        if (westOpen)
        {
            Node* node = getLandNode(current->x - 1, current->y, current->z + 1, verticalRange, floorY, Direction::SOUTH);
            if (node != NULL && !node->closed && node->distanceTo(target) < maxCost)
                neighbors->data[count++] = node;
        }
        if (eastOpen)
        {
            Node* node = getLandNode(current->x + 1, current->y, current->z + 1, verticalRange, floorY, Direction::SOUTH);
            if (node != NULL && !node->closed && node->distanceTo(target) < maxCost)
                neighbors->data[count++] = node;
        }
    }
    return count;
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z,
                                                           Mob* mob, int sizeX, int sizeY, int sizeZ,
                                                           bool canOpenDoors, bool canPassDoors)
{
    int pathTypeFlags = 0;
    const BlockPathTypes* pathType = getBlockPathTypes(levelSource, x, y, z, sizeX, sizeY, sizeZ, canOpenDoors,
                                                       canPassDoors, pathTypeFlags, BlockPathTypes::BLOCKED);
    if (pathTypeFlags & (1 << BlockPathTypes::FENCE->ordinal()))
        return BlockPathTypes::FENCE;

    const BlockPathTypes* worst = BlockPathTypes::BLOCKED;
    for (int i = 0; i < static_cast<int>(sizeof(BlockPathTypes::VALUES) / sizeof(BlockPathTypes::VALUES[0])); ++i)
    {
        const BlockPathTypes* type = BlockPathTypes::VALUES[i];
        if (!(pathTypeFlags & (1 << type->ordinal())))
            continue;
        if (mob->getPathfindingMalus(type) < 0.0f)
            return type;
        if (mob->getPathfindingMalus(type) >= mob->getPathfindingMalus(worst))
            worst = type;
    }
    if (pathType == BlockPathTypes::OPEN && mob->getPathfindingMalus(worst) == 0.0f)
        return BlockPathTypes::OPEN;
    return worst;
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathTypes(LevelSource* levelSource, int x, int y, int z,
                                                            int sizeX, int sizeY, int sizeZ,
                                                            bool canOpenDoors, bool canPassDoors,
                                                            int& pathTypeFlags, const BlockPathTypes* pathType)
{
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
                if (type == BlockPathTypes::DOOR_WOOD_CLOSED && canOpenDoors && canPassDoors)
                    type = BlockPathTypes::WALKABLE;
                else if (type == BlockPathTypes::DOOR_OPEN && !canPassDoors)
                    type = BlockPathTypes::BLOCKED;
                else if (type == BlockPathTypes::RAIL && !mRailAtStart && !mRailBelowStart)
                    type = BlockPathTypes::FENCE;
                if (dx == 0 && dy == 0 && dz == 0)
                    pathType = type;

                pathTypeFlags |= 1 << type->ordinal();
                mBlockPathTypeMap[key] = 1 << type->ordinal();
            }
    return pathType;
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathTypeRaw(LevelSource* levelSource, int x, int y, int z)
{
    BlockPos pos(x, y, z);
    const BlockState* state = getBlockState(mEntity->mTickCache, levelSource, pos);
    Block* block = state->getBlock();
    Material* material = state->getMaterial();
    if (material == Material::AIR)
        return BlockPathTypes::OPEN;
    if (material == Material::WATER)
        return BlockPathTypes::WATER;
    if (material == Material::LAVA)
        return BlockPathTypes::LAVA;
    if (block == Blocks::trapdoor || block == Blocks::spruce_trapdoor || block == Blocks::birch_trapdoor
        || block == Blocks::jungle_trapdoor || block == Blocks::acacia_trapdoor || block == Blocks::dark_oak_trapdoor
        || block == Blocks::iron_trapdoor || block == Blocks::waterlily)
        return BlockPathTypes::TRAPDOOR;
    if (block == Blocks::fire)
        return BlockPathTypes::DAMAGE_FIRE;
    if (block == Blocks::cactus)
        return BlockPathTypes::DAMAGE_CACTUS;
    if (block->mIsDoor)
    {
        if (DoorBlock::isOpen(levelSource, pos))
            return BlockPathTypes::DOOR_OPEN;
        if (material == Material::WOOD)
            return BlockPathTypes::DOOR_WOOD_CLOSED;
        if (material == Material::METAL)
            return BlockPathTypes::DOOR_IRON_CLOSED;
    }
    if (block->mIsRail)
        return BlockPathTypes::RAIL;
    if (block->mIsFence || block->mIsWall)
        return BlockPathTypes::FENCE;
    if (block->mIsFenceGate && !state->getValue(FenceGateBlock::OPEN))
        return BlockPathTypes::FENCE;
    if (!block->isPathfindable(levelSource, pos))
        return BlockPathTypes::BLOCKED;
    return BlockPathTypes::OPEN;
}

const BlockPathTypes* WalkNodeEvaluator::checkNeighbourBlocks(LevelSource* levelSource, int x, int y, int z,
                                                               const BlockPathTypes* pathType)
{
    if (pathType != BlockPathTypes::WALKABLE)
        return pathType;
    for (int dx = -1; dx <= 1; ++dx)
        for (int dz = -1; dz <= 1; ++dz)
        {
            if (dx == 0 && dz == 0)
                continue;
            int blockId = levelSource->getElement(mEntity->mTickCache, x + dx, y, z + dz) >> 4;
            if (blockId == BlockId::CACTUS)
                pathType = BlockPathTypes::DANGER_CACTUS;
            else if (blockId == BlockId::FIRE)
                pathType = BlockPathTypes::DANGER_FIRE;
        }
    return pathType;
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z)
{
    const BlockPathTypes* result = getBlockPathTypeRaw(levelSource, x, y, z);
    if (result == BlockPathTypes::OPEN && y >= 1)
    {
        int blockIdBelow = levelSource->getBlockId(mEntity->mTickCache, x, y - 1, z);
        const BlockPathTypes* below = getBlockPathTypeRaw(levelSource, x, y - 1, z);
        if (below != BlockPathTypes::WALKABLE && below != BlockPathTypes::OPEN && below != BlockPathTypes::WATER
            && below != BlockPathTypes::LAVA)
            result = BlockPathTypes::WALKABLE;
        else
            result = BlockPathTypes::OPEN;
        if (below == BlockPathTypes::DAMAGE_FIRE || blockIdBelow == BlockId::MAGMA)
            return checkNeighbourBlocks(levelSource, x, y, z, BlockPathTypes::DAMAGE_FIRE);
        if (below == BlockPathTypes::DAMAGE_CACTUS)
            result = BlockPathTypes::DAMAGE_CACTUS;
    }
    return checkNeighbourBlocks(levelSource, x, y, z, result);
}

Node* WalkNodeEvaluator::getLandNode(int x, int y, int z, int verticalRange, double maxFallDistance,
                                     const Direction* direction)
{
    Node* cachedNode = getNode(x, y, z);
    if (cachedNode->hasAcceptedNode)
        return cachedNode->acceptedNode;

    Node* node = NULL;
    BlockPos pos(x, y, z);
    BlockPos posBelow = pos.below();
    const double floorY = static_cast<double>(y)
                          - (1.0 - getBlockState(mEntity->mTickCache, mLevelSource, posBelow)
                                       ->getShape(mLevelSource, posBelow)->mMaxY);
    if (floorY - maxFallDistance > 1.125)
    {
        cachedNode->hasAcceptedNode = true;
        cachedNode->acceptedNode = NULL;
        return NULL;
    }

    const BlockPathTypes* type = getBlockPathType(mMob, x, y, z);
    float malus = mMob->getPathfindingMalus(type);
    const double halfWidth = mMob->mBbWidth * 0.5;
    if (malus >= 0.0f)
    {
        node = getNode(x, y, z);
        node->type = type;
        node->costMalus = std::max(node->costMalus, malus);
    }

    if (type != BlockPathTypes::WALKABLE)
    {
        if (node == NULL && verticalRange > 0 && type != BlockPathTypes::FENCE && type != BlockPathTypes::TRAPDOOR)
        {
            node = getLandNode(x, y + 1, z, verticalRange - 1, maxFallDistance, direction);
            if (node != NULL && (node->type == BlockPathTypes::OPEN || node->type == BlockPathTypes::WALKABLE)
                && mMob->mBbWidth < 1.0f)
            {
                const double centreX = (x - direction->getStepX()) + 0.5;
                const double centreZ = (z - direction->getStepZ()) + 0.5;
                AABB* space = AABB::newTemp(centreX - halfWidth, y + 0.001, centreZ - halfWidth, centreX + halfWidth,
                                            static_cast<float>(y) + mMob->mBbHeight, centreZ + halfWidth);
                const AABB* shape = getBlockState(mEntity->mTickCache, mLevelSource, pos)->getShape(mLevelSource, pos);
                if (mMob->mLevel->hasBlockCubes(space->expand(0.0, shape->mMaxY - 0.002, 0.0)))
                    node = NULL;
            }
        }

        if (type == BlockPathTypes::OPEN)
        {
            AABB* space = AABB::newTemp(x - halfWidth + 0.5, y + 0.001, z - halfWidth + 0.5, x + halfWidth + 0.5,
                                        static_cast<float>(y) + mMob->mBbHeight, z + halfWidth + 0.5);
            if (mMob->mLevel->hasBlockCubes(space))
            {
                cachedNode->hasAcceptedNode = true;
                cachedNode->acceptedNode = NULL;
                return NULL;
            }

            if (mMob->mBbWidth >= 1.0f && getBlockPathType(mMob, x, y - 1, z) == BlockPathTypes::BLOCKED)
            {
                node = getNode(x, y, z);
                cachedNode->acceptedNode = node;
                cachedNode->hasAcceptedNode = true;
                node->type = BlockPathTypes::WALKABLE;
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
                type = getBlockPathType(mMob, x, y, z);
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
                    cachedNode->acceptedNode = node;
                    node->type = type;
                    node->costMalus = std::max(node->costMalus, malus);
                    cachedNode->hasAcceptedNode = true;
                    return node;
                }
            }
        }
    }

    cachedNode->acceptedNode = node;
    cachedNode->hasAcceptedNode = true;
    return node;
}

void WalkNodeEvaluator::blockChanged(const BlockPos& pos)
{
    mEntity->mTickCache.blockChanged(pos);
}
