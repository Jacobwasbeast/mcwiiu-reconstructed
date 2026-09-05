#include "game/world/block/DynamicLiquidBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"

DynamicLiquidBlock::DynamicLiquidBlock(Material* material)
    : LiquidBlock(material), mTickQueue(), mProcessingTicks(false), mHighestDepth(0)
{
    LiquidBlock::DerivedInit();
}

DynamicLiquidBlock::~DynamicLiquidBlock()
{
}

void DynamicLiquidBlock::setStatic(Level* level, const BlockPos& pos, bool remove)
{
    const unsigned short element = LiquidBlock::getLiquidElement(level, pos);
    const unsigned short staticElement = static_cast<unsigned short>(LiquidBlock::getStaticLiquidElementId(element));
    const BlockState* state = level->getBlockState(pos);
    const int blockId = Block::getId(state->getBlock());
    if (remove || blockId == 8 || blockId == 10)
    {
        LiquidBlock::setLiquidElement(level, pos, staticElement, remove, 18);
    }
}

int DynamicLiquidBlock::getHighest(Level* level, const BlockPos& pos, int currentHighest)
{
    int depth = getDepth(level, pos);
    if (depth >= 0)
    {
        if (depth < 1)
        {
            ++mHighestDepth;
        }
        else if (depth > 7)
        {
            depth = 0;
        }
        if (currentHighest < 0 || depth < currentHighest)
        {
            return depth;
        }
    }
    return currentHighest;
}

bool DynamicLiquidBlock::_isWaterBlocking(Level* level, const BlockPos& pos,
                                           const BlockPos& sourcePos, const Direction* direction)
{
    const BlockState* state = level->getBlockState(pos);
    if (!state->isSolidRender()
        && state->getBlock()->liquidCanFlowIntoFromDirection(level, pos, direction))
    {
        const BlockState* sourceState = level->getBlockState(sourcePos);
        if (sourceState->getBlock()->liquidCanFlowIntoFromDirection(level, sourcePos,
                                                                      direction->getOpposite()))
        {
            return false;
        }
    }
    return true;
}

bool DynamicLiquidBlock::_canSpreadTo(Level* level, const BlockPos& pos,
                                      const BlockPos& sourcePos, const Direction* direction)
{
    if (!level->isInWorldBounds(pos))
    {
        return false;
    }
    const BlockState* state = level->getBlockState(pos);
    const Material* material = state->getMaterial();
    if (material != getCraftingMaterial() && material != Material::AIR)
    {
        return !_isWaterBlocking(level, pos, sourcePos, direction);
    }
    return false;
}

void DynamicLiquidBlock::_spread(Level* level, const BlockPos& pos, int depth, bool remove)
{
    unsigned short element = static_cast<unsigned short>((getCraftingMaterial() == Material::LAVA ? 10 : 8) << 4);
    element = static_cast<unsigned short>(element | depth);
    LiquidBlock::setLiquidElement(level, pos, element, remove, 3);
    level->addToTickNextTick(pos, this, getTickDelay(level));
}

void DynamicLiquidBlock::_trySpreadTo(Level* level, const BlockPos& pos, const BlockPos& sourcePos,
                                      const Direction* direction, int depth)
{
    if (!_canSpreadTo(level, pos, sourcePos, direction))
    {
        return;
    }
    const BlockState* state = level->getBlockState(pos);
    bool remove = true;
    if (getCraftingMaterial() == Material::LAVA)
    {
        state->getBlock()->handleRain(level, pos);
        _spread(level, pos, depth, false);
        return;
    }
    if (!state->getBlock()->isLiquidBlock())
    {
        level->destroyBlock(pos, true);
        level->levelEvent(2001, pos, Block::getIdWithData(state));
        remove = false;
    }
    _spread(level, pos, depth, remove);
}

int DynamicLiquidBlock::_getSlopeDistance(Level* level, const BlockPos& pos, int distance,
                                          int directionIndex)
{
    int best = 1000;
    static const Direction* directions[4] = {Direction::WEST, Direction::EAST,
                                             Direction::NORTH, Direction::SOUTH};
    for (int index = 0; index < 4; ++index)
    {
        if (index == directionIndex)
        {
            continue;
        }
        const BlockPos next = pos.relative(directions[index]);
        if (_isWaterBlocking(level, next, pos, directions[index]))
        {
            continue;
        }
        const BlockState* state = level->getBlockState(next);
        if (state->getMaterial() == getCraftingMaterial())
        {
            const BlockPos below = next.below();
            if (_isWaterBlocking(level, below, pos, Direction::DOWN))
            {
                const int limit = getCraftingMaterial() == Material::LAVA ? 2 : 4;
                if (distance < limit)
                {
                    const int slope = _getSlopeDistance(level, next, distance + 1, index);
                    if (slope < best)
                    {
                        best = slope;
                    }
                }
            }
        }
    }
    return best;
}

void DynamicLiquidBlock::_getSpread(Level* level, const BlockPos& pos, bool (&spread)[4])
{
    static const Direction* directions[4] = {Direction::WEST, Direction::EAST,
                                             Direction::NORTH, Direction::SOUTH};
    int distances[4] = {1000, 1000, 1000, 1000};
    int minimum = 1000;
    for (int index = 0; index < 4; ++index)
    {
        const BlockPos next = pos.relative(directions[index]);
        if (!_isWaterBlocking(level, next, pos, directions[index]))
        {
            const BlockPos below = next.below();
            if (_isWaterBlocking(level, below, pos, Direction::DOWN))
            {
                distances[index] = _getSlopeDistance(level, next, 1, index);
            }
        }
        if (distances[index] < minimum)
        {
            minimum = distances[index];
        }
    }
    for (int index = 0; index < 4; ++index)
    {
        spread[index] = distances[index] == minimum;
    }
}

void DynamicLiquidBlock::maintick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    const bool water = getCraftingMaterial() == Material::WATER;
    const bool remove = level->getBlockState(pos)->getMaterial() != getCraftingMaterial();
    const int oldDepth = getDepth(level, pos);
    const int rate = water ? 1 : 2;
    if (!water)
    {
        trySpreadFire(level, pos, random);
    }
    int depth = 1000;
    mHighestDepth = 0;
    static const Direction* directions[4] = {Direction::WEST, Direction::EAST,
                                             Direction::NORTH, Direction::SOUTH};
    for (int index = 0; index < 4; ++index)
    {
        depth = getHighest(level, pos.relative(directions[index]), depth);
    }
    depth += rate;
    const int aboveDepth = getDepth(level, pos.above());
    if (aboveDepth >= 0 && aboveDepth < 8)
    {
        depth = aboveDepth + 8;
    }
    if (mHighestDepth >= 2)
    {
        depth = -1;
    }
    if (oldDepth != depth)
    {
        if (depth < 0)
        {
            if (remove)
            {
                level->setBlockAndUpdate(pos, defaultBlockState());
            }
            else
            {
                level->removeBlock(pos);
            }
        }
        else
        {
            const unsigned short element = static_cast<unsigned short>(((water ? 8 : 10) << 4) | depth);
            LiquidBlock::setLiquidElement(level, pos, element, remove, 2);
            level->updateNeighborsAt(pos, this, false);
        }
    }
    if (depth >= 0)
    {
        const BlockPos below = pos.below();
        if (_canSpreadTo(level, below, pos, Direction::DOWN))
        {
            _trySpreadTo(level, below, pos, Direction::DOWN, depth < 8 ? depth + 8 : depth);
        }
        else if (oldDepth >= 0)
        {
            bool spread[4];
            _getSpread(level, pos, spread);
            const int nextDepth = oldDepth + rate < 8 ? oldDepth + rate : 1;
            for (int index = 0; index < 4; ++index)
            {
                if (spread[index])
                {
                    _trySpreadTo(level, pos.relative(directions[index]), pos, directions[index], nextDepth);
                }
            }
        }
    }
}

void DynamicLiquidBlock::iterativetick(Level* level, const BlockPos& pos,
                                       const BlockState* state, Random* random)
{
    LiquidTickData first;
    first.level = level;
    first.pos = pos;
    first.state = state;
    first.random = random;
    mTickQueue.push_back(first);
    for (int count = 100; !mTickQueue.empty() && count > 0; --count)
    {
        const LiquidTickData data = mTickQueue.front();
        mTickQueue.pop_front();
        maintick(data.level, data.pos, data.state, data.random);
    }
    mTickQueue.clear();
}

void DynamicLiquidBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!mProcessingTicks)
    {
        if (Level::getInstaTick())
        {
            mProcessingTicks = true;
            iterativetick(level, pos, state, random);
            mProcessingTicks = false;
            return;
        }
        if (!mProcessingTicks)
        {
            maintick(level, pos, state, random);
            return;
        }
    }
    if (Level::getInstaTick())
    {
        LiquidTickData data;
        data.level = level;
        data.pos = pos;
        data.state = state;
        data.random = random;
        mTickQueue.push_back(data);
    }
    else
    {
        maintick(level, pos, state, random);
    }
}

void DynamicLiquidBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!isTicking())
    {
        level->addToTickNextTick(pos, this, getTickDelay(level));
    }
}

bool DynamicLiquidBlock::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    return true;
}
