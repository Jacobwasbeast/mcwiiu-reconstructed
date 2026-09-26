#include "game/world/block/DynamicLiquidBlock.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TypedBoxed.hpp"

DynamicLiquidBlock::DynamicLiquidBlock(Material* material)
    : LiquidBlock(material), mTickQueue(), mProcessingTicks(false), mAdjacentSources(0)
{
    LiquidBlock::DerivedInit();
}

DynamicLiquidBlock::~DynamicLiquidBlock()
{
}

void DynamicLiquidBlock::setStatic(Level* level, const BlockPos& pos, bool extraLayer)
{
    unsigned short staticElement = static_cast<unsigned short>(LiquidBlock::getStaticLiquidElementId(
        LiquidBlock::getLiquidElement(level, pos)));
    int blockId = extraLayer ? level->getExtraBlockId(pos.x, pos.y, pos.z) : level->getBlockId(pos.x, pos.y, pos.z);
    if (blockId != BlockId::FLOWING_WATER)
    {
        if (blockId != BlockId::FLOWING_LAVA)
            return;
        extraLayer = false;
    }
    LiquidBlock::setLiquidElement(level, pos, staticElement, extraLayer, 18);
}

int DynamicLiquidBlock::getHighest(Level* level, const BlockPos& pos, int currentHighest)
{
    int depth = getDepth(level, pos);
    if (depth >= 0)
    {
        if (depth < 1)
        {
            ++mAdjacentSources;
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

bool DynamicLiquidBlock::_canSpreadTo(Level* level, const BlockPos& pos, const BlockPos& sourcePos,
                                      const Direction* direction)
{
    int size = level->mChunkCacheSize;
    int chunkX = (pos.x >> 4) + size / 2;
    int chunkZ = (pos.z >> 4) + size / 2;
    if (chunkX >= 0 && chunkX < size && chunkZ >= 0 && chunkZ < size)
    {
        Block* liquid = Block::byId(LiquidBlock::getLiquidElement(level, pos) >> 4);
        if (liquid->mMaterial != mMaterial && liquid->mMaterial != Material::LAVA)
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
        level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, pos, Block::getIdWithData(state));
        remove = false;
    }
    _spread(level, pos, depth, remove);
}

int DynamicLiquidBlock::_getSlopeDistance(Level* level, const BlockPos& pos, int distance, int directionIndex)
{
    static const Direction* fromDirections[4] = {Direction::EAST, Direction::WEST, Direction::SOUTH, Direction::NORTH};
    PIXBeginNamedEvent(0.0F, "getSlopeDistance");
    int best = 1000;
    for (int index = 0; index < 4; ++index)
    {
        if (index == (directionIndex ^ 1))
            continue;

        MutableBlockPos next(pos);
        if (index == 0)
            next.addX(-1);
        else if (index == 1)
            next.addX(1);
        else if (index == 2)
            next.addZ(-1);
        else
            next.addZ(1);
        if (_isWaterBlocking(level, next, pos, fromDirections[index]))
            continue;

        const BlockState* state = level->getBlockState(next);
        if (state->getBlock()->mMaterial == mMaterial && state->getValue(LEVEL) == 0)
            continue;

        if (!_isWaterBlocking(level, next.below(), pos, Direction::UP))
        {
            PIXEndNamedEvent();
            return distance;
        }

        int limit = mMaterial == Material::LAVA ? 2 : 4;
        if (distance < limit)
        {
            int slope = _getSlopeDistance(level, next, distance + 1, index);
            if (slope < best)
                best = slope;
        }
    }
    PIXEndNamedEvent();
    return best;
}

void DynamicLiquidBlock::_getSpread(Level* level, const BlockPos& pos, bool (&spread)[4])
{
    static const Direction* fromDirections[4] = {Direction::EAST, Direction::WEST, Direction::SOUTH, Direction::NORTH};
    PIXBeginNamedEvent(0.0F, "Get Spread");
    int distances[4];
    for (int index = 0; index < 4; ++index)
    {
        distances[index] = 1000;
        MutableBlockPos next(pos);
        if (index == 0)
            next.addX(-1);
        else if (index == 1)
            next.addX(1);
        else if (index == 2)
            next.addZ(-1);
        else
            next.addZ(1);
        if (_isWaterBlocking(level, next, pos, fromDirections[index]))
            continue;

        int element = level->getElement(next.x, next.y, next.z);
        Block* block = level->getBlock(next);
        if (block->mMaterial != mMaterial || (element & 0xf) != 0)
        {
            distances[index] = 0;
            if (_isWaterBlocking(level, next.below(), pos, Direction::UP))
                distances[index] = _getSlopeDistance(level, next, 1, index);
        }
    }

    int minimum = distances[0];
    if (distances[1] <= minimum)
        minimum = distances[1];
    if (distances[2] <= minimum)
        minimum = distances[2];
    if (distances[3] <= minimum)
        minimum = distances[3];
    spread[0] = distances[0] == minimum;
    spread[1] = distances[1] == minimum;
    spread[2] = distances[2] == minimum;
    spread[3] = distances[3] == minimum;
    PIXEndNamedEvent();
}

void DynamicLiquidBlock::maintick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    bool extraLayer = level->getBlockState(pos)->getMaterial() != mMaterial;
    int depth = getDepth(level, pos);
    int dropOff = 1;
    if (mMaterial == Material::LAVA)
    {
        if (!level->mDimension->isUltraWarm())
            dropOff = 2;
        trySpreadFire(level, pos, random);
    }
    int tickDelay = getTickDelay(level);

    bool placeStatic = false;
    if (depth < 1)
    {
        placeStatic = true;
    }
    else
    {
        PIXBeginNamedEvent(0.0F, "Tick depth > 0");
        int highest = -100;
        mAdjacentSources = 0;
        arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
        for (unsigned int i = 0; i < directions.length; ++i)
            highest = getHighest(level, pos.relative(directions.data[i]), highest);

        int newDepth = highest + dropOff;
        if (newDepth > 7 || highest < 0)
            newDepth = -1;

        int aboveDepth = getDepth(level, pos.above());
        if (aboveDepth >= 0)
            newDepth = aboveDepth < 8 ? aboveDepth + 8 : aboveDepth;

        if (mAdjacentSources == 1 && getDepth(level, pos.above()) == 0)
            ++mAdjacentSources;

        if (mAdjacentSources >= 2 && mMaterial == Material::WATER)
        {
            const BlockState* belowState = level->getBlockState(pos.below());
            if (belowState->getMaterial()->isSolid())
            {
                newDepth = 0;
            }
            else if (belowState->getBlock()->mMaterial == mMaterial && belowState->getValue(LEVEL) == 0)
            {
                newDepth = 0;
            }
        }

        if (mMaterial == Material::LAVA && depth < 8 && newDepth < 8 && newDepth > depth && random->nextInt(4) != 0)
            tickDelay *= 4;

        if (newDepth == depth)
        {
            placeStatic = true;
            PIXEndNamedEvent();
        }
        else
        {
            depth = newDepth;
            if (newDepth < 0)
            {
                if (extraLayer)
                    level->setExtraBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
                else
                    level->removeBlock(pos);
            }
            else
            {
                TypedBoxed<int> levelValue(newDepth);
                const BlockState* newState = state->setBoxedValue(LEVEL, &levelValue);
                if (mMaterial == Material::LAVA)
                {
                    extraLayer = false;
                    level->setBlock(pos, newState, Block::UPDATE_CLIENTS, false);
                }
                else if (extraLayer)
                {
                    level->setExtraBlock(pos, newState, Block::UPDATE_CLIENTS, false);
                }
                else
                {
                    level->setBlock(pos, newState, Block::UPDATE_CLIENTS, false);
                }
                level->addToTickNextTick(pos, this, tickDelay);
                level->updateNeighborsAt(pos, this, false);
            }
            PIXEndNamedEvent();
        }
    }

    if (placeStatic)
    {
        PIXBeginNamedEvent(0.0F, "Tick setStatic");
        setStatic(level, pos, extraLayer);
        PIXEndNamedEvent();
    }

    PIXBeginNamedEvent(0.0F, "Check for flow and spread");
    if (_canSpreadTo(level, pos.below(), pos, Direction::UP))
    {
        PIXBeginNamedEvent(0.0F, "Can flow");
        if (mMaterial == Material::LAVA
            && (!Minecraft::InMiniGame(eMiniGameId_Any, true)
                || Minecraft::GetInstance()->GetMiniGame()->canLavaConvertWater()))
        {
            Material* belowMaterial = level->getBlockState(pos.below())->getMaterial();
            if (belowMaterial == Material::WATER || belowMaterial == Material::BUBBLE_COLUMN)
            {
                level->setBlockAndUpdate(pos.below(), Blocks::stone->defaultBlockState());
                fizz(level, pos.below());
                PIXEndNamedEvent();
                PIXEndNamedEvent();
                return;
            }
        }
        _trySpreadTo(level, pos.below(), pos, Direction::UP, depth < 8 ? depth + 8 : depth);
        PIXEndNamedEvent();
    }
    else if (depth >= 0 && (depth == 0 || _isWaterBlocking(level, pos.below(), pos, Direction::UP)))
    {
        PIXBeginNamedEvent(0.0F, "Cannot flow");
        bool spread[4];
        _getSpread(level, pos, spread);
        int spreadDepth = depth + dropOff;
        if (depth >= 8)
            spreadDepth = 1;
        if (spreadDepth <= 7)
        {
            if (spread[0])
                _trySpreadTo(level, pos.west(), pos, Direction::EAST, spreadDepth);
            if (spread[1])
                _trySpreadTo(level, pos.east(), pos, Direction::WEST, spreadDepth);
            if (spread[2])
                _trySpreadTo(level, pos.north(), pos, Direction::SOUTH, spreadDepth);
            if (spread[3])
                _trySpreadTo(level, pos.south(), pos, Direction::NORTH, spreadDepth);
        }
        PIXEndNamedEvent();
    }
    PIXEndNamedEvent();
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
