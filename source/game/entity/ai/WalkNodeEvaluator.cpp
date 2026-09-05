#include "game/entity/ai/WalkNodeEvaluator.hpp"

#include <cmath>

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

WalkNodeEvaluator::WalkNodeEvaluator(Entity* entity)
    : NodeEvaluator(), mOldWaterCost(1.65f), mFlag0(false), mFlag1(false), mEntity(entity)
{
}

void WalkNodeEvaluator::prepare(LevelSource* levelSource, Mob* mob)
{
    NodeEvaluator::prepare(levelSource, mob);
    mOldWaterCost = mob->getPathfindingMalus(BlockPathTypes::WATER);
    mFlag0 = false;
    mFlag1 = false;
}

void WalkNodeEvaluator::done()
{
    mMob->setPathfindingMalus(BlockPathTypes::WATER, mOldWaterCost);
    NodeEvaluator::done();
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathType(Mob* mob, int x, int y, int z)
{
    return getBlockPathType(mLevelSource, x, y, z, mob, mEntityWidth, mEntityHeight, mEntityY,
                            isCanOpenDoors(), isCanPassDoors());
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathType(Mob* mob, const BlockPos& pos)
{
    return getBlockPathType(mob, pos.x, pos.y, pos.z);
}

Node* WalkNodeEvaluator::getStart()
{
    int x = static_cast<int>(mMob->mX);
    int z = static_cast<int>(mMob->mZ);
    if (mMob->mX < static_cast<double>(x)) --x;
    if (mMob->mZ < static_cast<double>(z)) --z;
    int y = static_cast<int>(mMob->mY);
    if (mMob->mY < static_cast<double>(y)) --y;
    return NodeEvaluator::getNode(x, y, z);
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
    const Direction* directions[4] = { Direction::NORTH, Direction::SOUTH, Direction::WEST, Direction::EAST };
    int i;
    for (i = 0; i < 4; ++i)
    {
        const Direction* direction = directions[i];
        Node* node = getLandNode(current->x + direction->getStepX(), current->y,
                                 current->z + direction->getStepZ(), mEntityY, maxCost, direction);
        if (node != NULL && !node->closed && node->distanceTo(target) < maxCost)
            neighbors->data[count++] = node;
    }
    return count;
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z,
                                                           Mob* mob, int sizeX, int sizeY, int sizeZ,
                                                           bool canOpenDoors, bool canPassDoors)
{
    int flags = 0;
    const BlockPathTypes* result = BlockPathTypes::BLOCKED;
    return getBlockPathTypes(levelSource, x, y, z, sizeX, sizeY, sizeZ,
                             canOpenDoors, canPassDoors, flags, result);
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathTypes(LevelSource* levelSource, int x, int y, int z,
                                                            int sizeX, int sizeY, int sizeZ,
                                                            bool canOpenDoors, bool canPassDoors,
                                                            int& pathTypeFlags, const BlockPathTypes* preferred)
{
    const BlockPathTypes* result = preferred;
    int dx, dy, dz;
    pathTypeFlags = 0;
    for (dx = 0; dx < sizeX; ++dx)
        for (dy = 0; dy < sizeY; ++dy)
            for (dz = 0; dz < sizeZ; ++dz)
            {
                const BlockPathTypes* type = getBlockPathType(levelSource, x + dx, y + dy, z + dz);
                pathTypeFlags |= 1 << (type->ordinal() & 31);
                if (dx == 0 && dy == 0 && dz == 0)
                    result = type;
                else if (type == BlockPathTypes::FENCE)
                    return type;
                else if (type == BlockPathTypes::BLOCKED && result == BlockPathTypes::OPEN)
                    result = type;
                else if (type == BlockPathTypes::WALKABLE && result == BlockPathTypes::OPEN)
                    result = type;
            }
    return result;
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathTypeRaw(LevelSource* levelSource, int x, int y, int z)
{
    BlockPos pos(x, y, z);
    const BlockState* state = levelSource->getBlockState(mEntity->mTickCache, pos);
    Material* material = state->getMaterial();
    Block* block = state->getBlock();
    if (material == Material::WATER)
        return BlockPathTypes::WATER;
    if (material == Material::LAVA)
        return BlockPathTypes::LAVA;
    if (material == Material::AIR)
        return BlockPathTypes::OPEN;
    if (block->isPathfindable(levelSource, pos))
        return BlockPathTypes::WALKABLE;
    if (DoorBlock::isOpen(levelSource, pos))
        return BlockPathTypes::DOOR_OPEN;
    if (material == Material::FIRE)
        return BlockPathTypes::DAMAGE_FIRE;
    return BlockPathTypes::BLOCKED;
}

const BlockPathTypes* WalkNodeEvaluator::checkNeighbourBlocks(LevelSource* levelSource, int x, int y, int z,
                                                               const BlockPathTypes* pathType)
{
    if (pathType != BlockPathTypes::OPEN)
        return pathType;
    const int offsets[3] = { -1, 0, 1 };
    int i, j;
    for (i = 0; i < 3; ++i)
        for (j = 0; j < 3; ++j)
        {
            const BlockPathTypes* type = getBlockPathTypeRaw(levelSource, x + offsets[i], y, z + offsets[j]);
            if (type == BlockPathTypes::DAMAGE_FIRE)
                return BlockPathTypes::DANGER_FIRE;
            if (type == BlockPathTypes::LAVA)
                return BlockPathTypes::DANGER_OTHER;
        }
    return pathType;
}

const BlockPathTypes* WalkNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z)
{
    const BlockPathTypes* result = getBlockPathTypeRaw(levelSource, x, y, z);
    if (result == BlockPathTypes::BLOCKED && y > 0)
    {
        const BlockPathTypes* below = getBlockPathTypeRaw(levelSource, x, y - 1, z);
        if (below == BlockPathTypes::OPEN || below == BlockPathTypes::WALKABLE || below == BlockPathTypes::WATER)
            result = below;
    }
    return checkNeighbourBlocks(levelSource, x, y, z, result);
}

Node* WalkNodeEvaluator::getLandNode(int x, int y, int z, int verticalRange, double maxFallDistance,
                                     const Direction* direction)
{
    const BlockPathTypes* type = getBlockPathType(mMob, x, y, z);
    if (type == BlockPathTypes::BLOCKED)
        return NULL;
    if (type == BlockPathTypes::OPEN && y > 0)
    {
        int depth = 0;
        while (depth < verticalRange && getBlockPathType(mMob, x, y - 1, z) == BlockPathTypes::OPEN)
        {
            --y;
            ++depth;
        }
        type = getBlockPathType(mMob, x, y, z);
    }
    if (type == BlockPathTypes::BLOCKED)
        return NULL;
    Node* node = NodeEvaluator::getNode(x, y, z);
    return node;
}

void WalkNodeEvaluator::blockChanged(const BlockPos& pos)
{
    mEntity->mTickCache.blockChanged(pos);
}
