#include "game/entity/ai/SwimNodeEvaluator.hpp"

#include "game/core/Direction.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/BlockPos.hpp"

SwimNodeEvaluator::SwimNodeEvaluator(bool allowBreaching)
    : NodeEvaluator(),
      mAllowBreaching(allowBreaching)
{
}

const BlockPathTypes* SwimNodeEvaluator::getPathTypeAtPos(int x, int y, int z, bool checkNeighbors)
{
    BlockPos pos(x, y, z);
    const BlockState* state = mLevelSource->getBlockState(mMob->mTickCache, pos);
    const Material* material = state->getMaterial();
    const Material* liquidMaterial = Material::AIR;
    if (material == Material::AIR)
        liquidMaterial = mLevelSource->getLiquidBlock(pos)->getCraftingMaterial();

    if (material == Material::AIR && liquidMaterial == Material::AIR)
    {
        BlockPos belowPos = pos.below();
        const BlockState* belowState = mLevelSource->getBlockState(mMob->mTickCache, belowPos);
        if (belowState->getMaterial() == Material::WATER)
            return BlockPathTypes::BREACH;
        if (static_cast<double>(y) <= mMob->mY)
            return BlockPathTypes::WALKABLE;
    }
    else
    {
        if (material == Material::WATER)
        {
            if (checkNeighbors)
            {
                for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
                     it != Direction::VALUES.end(); ++it)
                {
                    const Direction* direction = *it;
                    if (direction != Direction::DOWN && direction != Direction::UP &&
                        getPathTypeAtPos(x + direction->getStepX(), y + direction->getStepY(),
                                         z + direction->getStepZ(), false) == BlockPathTypes::BLOCKED)
                        return BlockPathTypes::WATER_BORDER;
                }
            }
            return BlockPathTypes::WATER;
        }

        Block* block = state->getBlock();
        if (block->isSlabBlock() && HalfSlabBlock::isBottomSlab(block->convertBlockStateToLegacyData(state)))
            return BlockPathTypes::WALKABLE;
    }
    return BlockPathTypes::BLOCKED;
}

Node* SwimNodeEvaluator::getStart()
{
    int x = static_cast<int>(mMob->mX);
    if (mMob->mX < static_cast<double>(x))
        --x;
    int y = static_cast<int>(mMob->mY + 0.5);
    if (mMob->mY + 0.5 < static_cast<double>(y))
        --y;
    int z = static_cast<int>(mMob->mZ);
    if (mMob->mZ < static_cast<double>(z))
        --z;
    return NodeEvaluator::getNode(x, y, z);
}

Node* SwimNodeEvaluator::getGoal(double x, double y, double z)
{
    double offset = static_cast<double>(mMob->mBbWidth * 0.5f);
    x -= offset;
    int ix = static_cast<int>(x);
    if (x < static_cast<double>(ix))
        --ix;
    int iy = static_cast<int>(y + 0.5);
    if (y + 0.5 < static_cast<double>(iy))
        --iy;
    z -= offset;
    int iz = static_cast<int>(z);
    if (z < static_cast<double>(iz))
        --iz;
    return NodeEvaluator::getNode(ix, iy, iz);
}

Node* SwimNodeEvaluator::getDirectGoal(const BlockPos& pos)
{
    return NodeEvaluator::getNode(pos.x, pos.y, pos.z);
}

const BlockPathTypes* SwimNodeEvaluator::isFree(int x, int y, int z)
{
    for (int xx = x; xx < x + mEntityWidth; ++xx)
        for (int yy = y; yy < y + mEntityHeight; ++yy)
            for (int zz = z; zz < z + mEntityY; ++zz)
            {
                const BlockPathTypes* type = getPathTypeAtPos(xx, yy, zz, true);
                if (type != BlockPathTypes::WATER)
                    return type;
            }
    return BlockPathTypes::WATER;
}

Node* SwimNodeEvaluator::getWaterNode(int x, int y, int z)
{
    const BlockPathTypes* type = isFree(x, y, z);
    if (type == BlockPathTypes::WATER || (mAllowBreaching && type == BlockPathTypes::BREACH))
        return NodeEvaluator::getNode(x, y, z);

    if (!mMob->isInWaterOrBubble())
    {
        if (type == BlockPathTypes::WATER_BORDER || type == BlockPathTypes::WALKABLE)
            return NodeEvaluator::getNode(x, y, z);
    }
    else if (type == BlockPathTypes::WATER_BORDER)
    {
        int mobX = static_cast<int>(mMob->mX);
        if (mMob->mX < static_cast<double>(mobX))
            --mobX;
        int mobY = static_cast<int>(mMob->mY);
        if (mMob->mY < static_cast<double>(mobY))
            --mobY;
        int mobZ = static_cast<int>(mMob->mZ);
        if (mMob->mZ < static_cast<double>(mobZ))
            --mobZ;
        if (getPathTypeAtPos(mobX, mobY, mobZ, false) == BlockPathTypes::WATER)
            return NodeEvaluator::getNode(x, y, z);
    }
    return NULL;
}

int SwimNodeEvaluator::getNeighbors(arrayWithLength<Node*>* neighbors, Node* current, Node* target, float maxCost)
{
    int count = 0;
    for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
         it != Direction::VALUES.end(); ++it)
    {
        const Direction* direction = *it;
        Node* node = getWaterNode(current->x + direction->getStepX(),
                                  current->y + direction->getStepY(),
                                  current->z + direction->getStepZ());
        if (node != NULL && !node->closed && node->distanceTo(target) < maxCost)
        {
            neighbors->data[count++] = node;
        }
    }
    return count;
}

const BlockPathTypes* SwimNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z, Mob* mob,
                                                           int sizeX, int sizeY, int sizeZ,
                                                           bool canOpenDoors, bool canPassDoors)
{
    return BlockPathTypes::WATER;
}

const BlockPathTypes* SwimNodeEvaluator::getBlockPathType(LevelSource* levelSource, int x, int y, int z)
{
    return BlockPathTypes::WATER;
}

void SwimNodeEvaluator::blockChanged(const BlockPos& pos)
{
}
