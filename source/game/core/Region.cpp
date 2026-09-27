#include "game/core/Region.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/PIXNamedEvents.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/entity/mob/EntityTickCache.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/Blocks.hpp"

namespace
{
    const BlockState* emptyState()
    {
        return Blocks::air->defaultBlockState();
    }
}

Region::Region(Level* level, const BlockPos& from, const BlockPos& to, int expansion)
    : mMinChunkX((from.x - expansion) >> 4),
      mMinChunkZ((from.z - expansion) >> 4),
      mChunks(new array2DWithLength<LevelChunk*>(((to.x + expansion) >> 4) - mMinChunkX + 1,
                                                   ((to.z + expansion) >> 4) - mMinChunkZ + 1)),
      mLevel(level),
      mAllEmpty(true),
      mCachedX(-1),
      mCachedY(0),
      mCachedZ(-1),
      mCachedStates(NULL)
{
    int maxX = (to.x + expansion) >> 4;
    int maxZ = (to.z + expansion) >> 4;
    int x;
    for (x = mMinChunkX; x <= maxX; ++x)
    {
        int z;
        for (z = mMinChunkZ; z <= maxZ; ++z)
            mChunks->data[x - mMinChunkX]->data[z - mMinChunkZ] = level->getChunk(x, z);
    }
    for (x = from.x >> 4; x <= to.x >> 4; ++x)
    {
        int z;
        for (z = from.z >> 4; z <= to.z >> 4; ++z)
        {
            LevelChunk* chunk = getChunk(x, z);
            if (chunk != NULL && !chunk->isYSpaceEmpty(from.y, to.y))
                mAllEmpty = false;
        }
    }
    mCachedX = -1;
    mCachedStates = NULL;
    mCachedZ = -1;
}

Region::~Region()
{
    if (mChunks != NULL)
    {
        unsigned int i;
        for (i = 0; i < mChunks->length; ++i)
        {
            delete[] mChunks->data[i]->data;
            delete mChunks->data[i];
        }
        delete[] mChunks->data;
        delete mChunks;
    }
}

LevelChunk* Region::getChunk(int x, int z) const
{
    int ix = x - mMinChunkX;
    int iz = z - mMinChunkZ;
    if (ix < 0 || iz < 0 || mChunks == NULL || (unsigned int)ix >= mChunks->length)
        return NULL;
    if ((unsigned int)iz >= mChunks->data[ix]->length)
        return NULL;
    return mChunks->data[ix]->data[iz];
}

boost::shared_ptr<BlockEntity> Region::getBlockEntity(const BlockPos& pos)
{
    LevelChunk* chunk = getChunk(pos.x >> 4, pos.z >> 4);
    return chunk == NULL ? boost::shared_ptr<BlockEntity>() : chunk->getBlockEntity(pos, LevelChunk::IMMEDIATE);
}

boost::shared_ptr<BlockEntity> Region::getBlockEntity(const BlockPos& pos, LevelChunk::EntityCreationType type)
{
    LevelChunk* chunk = getChunk(pos.x >> 4, pos.z >> 4);
    return chunk == NULL ? boost::shared_ptr<BlockEntity>() : chunk->getBlockEntity(pos, type);
}

int Region::getRawBrightness(const BlockPos& pos, bool useNeighbourBrightness)
{
    if (!mLevel->isWithinLevelBounds(pos.x, pos.z))
        return 15;
    if (useNeighbourBrightness && getBlockState(pos)->doesPropagate())
    {
        int brightest = 0;
        const Direction* directions[6] = { Direction::DOWN, Direction::UP, Direction::NORTH, Direction::SOUTH, Direction::WEST, Direction::EAST };
        int i;
        for (i = 0; i < 6; ++i)
        {
            int value = getRawBrightness(pos.relative(directions[i]), false);
            if (value > brightest) brightest = value;
            if (brightest > 14) return brightest;
        }
        return brightest;
    }
    if (pos.y < 0) return 0;
    if (pos.y < 256)
    {
        LevelChunk* chunk = getChunk(pos.x >> 4, pos.z >> 4);
        return chunk == NULL ? 0 : chunk->getRawBrightness(pos, mLevel->getSkyDarken());
    }
    int value = 15 - mLevel->getSkyDarken();
    return value < 0 ? 0 : value;
}

int Region::getRawBrightness(const BlockPos& pos) { return getRawBrightness(pos, true); }
float Region::getBrightness(const BlockPos& pos, int lightValue)
{
    int value = getRawBrightness(pos);
    if (value < lightValue) value = lightValue;
    return mLevel->mDimension->getBrightnessRamp()[value];
}

int Region::getBrightnessPropagate(LightLayer::variety layer, const BlockPos& pos, int lightValue)
{
    if (layer == LightLayer::SKY && !mLevel->mDimension->isHasSkyLight())
        return 0;

    BlockPos target = pos.y < 0 ? BlockPos(pos.x, 0, pos.z) : pos;
    if (!mLevel->reallyHasChunkAt(target))
        return layer;

    if (!getBlockState(target)->doesPropagate())
        return getChunk(target.x >> 4, target.z >> 4)->getBrightness(layer, target);

    int brightest = 0;
    const Direction* directions[6] = { Direction::DOWN, Direction::UP, Direction::NORTH,
                                       Direction::SOUTH, Direction::WEST, Direction::EAST };
    for (int i = 0; i < 6; ++i)
    {
        int value = getBrightness(layer, target.relative(directions[i]));
        if (value > brightest)
        {
            brightest = value;
            if (brightest > 14)
                return brightest;
        }
    }
    return brightest;
}

int Region::getLightColor(const BlockPos& pos, int lightValue, int cacheIndex)
{
    int sky = getBrightnessPropagate(LightLayer::SKY, pos, cacheIndex);
    int block = getBrightnessPropagate(LightLayer::BLOCK, pos, cacheIndex);
    if (block < lightValue) block = lightValue;
    return (sky << 20) | (block << 4);
}

float Region::getBrightness(const BlockPos& pos) { return mLevel->mDimension->getBrightnessRamp()[getRawBrightness(pos)]; }

const BlockState* Region::getBlockState(const BlockPos& pos)
{
    if (pos.y < 0 || pos.y >= 256) return emptyState();
    if (mCachedStates != NULL && pos.x >> 4 == mCachedX && pos.z >> 4 == mCachedZ && pos.y >= (mCachedY * 16 - 1) && pos.y < mCachedY * 16 + 17)
        return mCachedStates[((pos.y - (mCachedY * 16 - 1)) << 8) | ((pos.z & 15) << 4) | (pos.x & 15)];
    LevelChunk* chunk = getChunk(pos.x >> 4, pos.z >> 4);
    return chunk == NULL ? emptyState() : chunk->getBlockState(pos);
}

const BlockState* Region::getBlockState(EntityTickCache& tickCache, const BlockPos& pos)
{
    if (pos.y < 0 || pos.y >= 256) return emptyState();
    const BlockState* state = tickCache.getBlockState(pos.x, pos.y, pos.z);
    return state == NULL ? getBlockState(pos) : state;
}

const BlockState* Region::getBlockState(LevelChunk* chunk, EntityTickCache&, const BlockPos& pos)
{
    return chunk == NULL ? getBlockState(pos) : chunk->getBlockState(pos);
}

int Region::getBlockId(int x, int y, int z)
{
    if ((unsigned int)y < 256)
    {
        if (mCachedStates != NULL && x >> 4 == mCachedX && z >> 4 == mCachedZ)
        {
            int yBase = mCachedY * 16 - 1;
            if (y >= yBase && y < mCachedY * 16 + 17)
                return Block::getId(mCachedStates[((y - yBase) << 8) | ((z & 15) << 4) | (x & 15)]->getBlock());
        }
        LevelChunk* chunk = getChunk(x >> 4, z >> 4);
        if (chunk != NULL)
            return chunk->getBlockId(x, y, z);
    }
    return 0;
}

int Region::getBlockId(EntityTickCache& cache, int x, int y, int z)
{
    int element = cache.getElement(x, y, z);
    if (element == 0xffff)
        element = getElement(x, y, z);
    return element >> 4;
}

void Region::getBlockAndExtraBlockId(int x, int y, int z, int* blockId, int* extraBlockId)
{
    *blockId = getBlockId(x, y, z);
    *extraBlockId = getExtraBlockId(x, y, z);
}

unsigned short Region::getElement(EntityTickCache& cache, int x, int y, int z)
{
    unsigned short element = cache.getElement(x, y, z);
    if (element == 0xffff)
        element = getElement(x, y, z);
    return element;
}

unsigned short Region::getElement(int x, int y, int z)
{
    if ((unsigned int)y < 256)
    {
        LevelChunk* chunk = getChunk(x >> 4, z >> 4);
        if (chunk != NULL)
            return chunk->getElement(x, y, z);
    }
    return 0;
}

int Region::getBlockData(int x, int y, int z)
{
    if (y >= 0 && y < 256)
    {
        if (mCachedStates != NULL && x >> 4 == mCachedX && z >> 4 == mCachedZ)
        {
            int yBase = mCachedY * 16 - 1;
            if (y >= yBase && y < yBase + 17)
            {
                const BlockState* state = mCachedStates[((y - yBase) << 8) | ((z & 15) << 4) | (x & 15)];
                Block* block = state->getBlock();
                return block->convertBlockStateToLegacyData(state);
            }
        }
        LevelChunk* chunk = getChunk(x >> 4, z >> 4);
        if (chunk != NULL)
            return chunk->getData(x & 15, y, z & 15);
    }
    return 0;
}
unsigned short Region::getExtraElement(int x, int y, int z)
{
    if ((unsigned int)y < 256)
    {
        LevelChunk* chunk = getChunk(x >> 4, z >> 4);
        if (chunk != NULL)
            return chunk->getExtraElement(x, y, z);
    }
    return 0;
}

void Region::getElementAndExtraElement(int x, int y, int z, unsigned short* element, unsigned short* extraElement)
{
    if ((unsigned int)y < 256)
    {
        LevelChunk* chunk = getChunk(x >> 4, z >> 4);
        if (chunk != NULL)
            chunk->getElementAndExtraElement(x, y, z, element, extraElement);
    }
    else
    {
        *element = 0;
        *extraElement = 0;
    }
}

Block* Region::getBlock(const BlockPos& pos)
{
    if (pos.y >= 0 && pos.y < 256)
    {
        if (mCachedStates != NULL && pos.x >> 4 == mCachedX && pos.z >> 4 == mCachedZ)
        {
            int yBase = mCachedY * 16 - 1;
            if (pos.y >= yBase && pos.y < yBase + 17)
            {
                const BlockState* state = mCachedStates[((pos.y - yBase) << 8) | ((pos.z & 15) << 4) | (pos.x & 15)];
                return state->getBlock();
            }
        }
        LevelChunk* chunk = getChunk(pos.x >> 4, pos.z >> 4);
        if (chunk != NULL)
            return chunk->getBlock(pos);
    }
    return Blocks::air;
}
void Region::getBlockAndExtraBlock(const BlockPos& pos, Block** block, Block** extraBlock) { *block = getBlock(pos); *extraBlock = getExtraBlock(pos); }
bool Region::isSolidRender(const BlockPos& pos)
{
    const BlockState* state = getBlockState(pos);

    if (state == NULL)
    {
        return false;
    }

    if (!LeafBlock::isLeaves(state->getBlock()))
    {
        return state->getBlock()->isSolidRender(state);
    }

    int xOffsets[6] = { 1, -1, 0, 0, 0, 0 };
    int yOffsets[6] = { 0, 0, 1, -1, 0, 0 };
    int zOffsets[6] = { 0, 0, 0, 0, 1, -1 };

    for (int i = 0; i < 6; ++i)
    {
        const BlockState* neighbour = mLevel->getBlockState(pos.offset(xOffsets[i], yOffsets[i], zOffsets[i]));

        if (!LeafBlock::isLeaves(neighbour->getBlock()) && !neighbour->isSolidRender())
        {
            return false;
        }
    }

    return true;
}
Biome* Region::getBiome(const BlockPos& pos)
{
    LevelChunk* chunk = mChunks->data[(pos.x >> 4) - mMinChunkX]->data[(pos.z >> 4) - mMinChunkZ];
    return chunk->getBiome(pos, mLevel->getBiomeSource());
}

BiomeSource* Region::getBiomeSource() { return mLevel->getBiomeSource(); }
int Region::getMaxBuildHeight() { return 256; }
bool Region::isAllEmpty() { return mAllEmpty; }
int Region::getDirectSignal(const BlockPos& pos, const Direction* direction)
{
    return getBlockState(pos)->getDirectSignal(this, pos, direction);
}

LevelType* Region::getGeneratorType() { return mLevel->getGeneratorType(); }
int Region::getExtraBlockId(int x, int y, int z) { return getExtraElement(x, y, z) >> 4; }
int Region::getExtraBlockData(int x, int y, int z) { return getExtraElement(x, y, z) & 15; }
Block* Region::getExtraBlock(const BlockPos& pos)
{
    if ((unsigned int)pos.y < 256)
    {
        LevelChunk* chunk = getChunk(pos.x >> 4, pos.z >> 4);
        if (chunk != NULL)
            return chunk->getExtraBlock(pos);
    }
    return Blocks::air;
}

Block* Region::getLiquidBlock(const BlockPos& pos)
{
    Block* extra = getExtraBlock(pos);
    if (extra != NULL && extra != Blocks::air)
        return extra;
    return getBlock(pos);
}

const BlockState* Region::getLiquidBlockState(const BlockPos& pos)
{
    const BlockState* extra = getExtraBlockState(pos);
    if (extra != NULL && extra != emptyState())
        return extra;
    return getBlockState(pos);
}
const BlockState* Region::getExtraBlockState(const BlockPos& pos)
{
    if (pos.y >= 0 && pos.y < 256)
    {
        LevelChunk* chunk = getChunk(pos.x >> 4, pos.z >> 4);
        if (chunk != NULL)
            return chunk->getExtraBlockState(pos);
    }
    return emptyState();
}
bool Region::isEmptyBlock(const BlockPos& pos) { return getBlockId(pos.x, pos.y, pos.z) == BlockId::AIR; }
bool Region::isEmptyWaterBlock(const BlockPos& pos) { return getBlockId(pos.x, pos.y, pos.z) == BlockId::WATER; }
int Region::getBrightness(LightLayer::variety layer, const BlockPos& pos) { if (pos.y < 0 || pos.y >= 256) return layer; LevelChunk* c = getChunk(pos.x >> 4, pos.z >> 4); return c == NULL ? layer : c->getBrightness(layer, pos); }
void Region::setCachedBlocksAndData(unsigned short* elements, int x, int y, int z, const BlockState** states)
{
    mCachedX = x;
    mCachedY = y;
    mCachedZ = z;
    mCachedStates = states;

    PIXBeginNamedEvent(0.0f, "Region::setCachedBlocksAndData");

    const BlockState* airState = emptyState();
    int lastBlockId = 0xffff;
    int lastData = -1;
    const BlockState* lastState = NULL;
    int yBase = y * 16 - 1;

    int yi;
    for (yi = 0; yi < 18; ++yi)
    {
        int worldY = yi + yBase;
        int xi;
        for (xi = 0; xi < 16; ++xi)
        {
            int zi;
            for (zi = 0; zi < 16; ++zi)
            {
                const BlockState* state;
                if (worldY >= 0 && worldY < 256)
                {
                    int index = ((worldY & 0x80) << 8) | (xi << 11) | (zi << 7) | (worldY & 0x7f);
                    unsigned short element = elements[index];
                    int blockId = elementToBlockId(element);
                    int data = elementToData(element);
                    if (blockId == BlockId::AIR)
                    {
                        state = airState;
                    }
                    else
                    {
                        if (blockId != lastBlockId || data != lastData)
                        {
                            Block* block = Block::byId(blockId);
                            lastState = block->getBlockState(data);
                            lastBlockId = blockId;
                            lastData = data;
                        }
                        state = lastState;
                    }
                }
                else
                {
                    state = airState;
                }
                states[(yi << 8) | (zi << 4) | xi] = state;
            }
        }
    }

    PIXEndNamedEvent();
}
unsigned int Region::elementToBlockId(unsigned short element) { return (element >> 4) & 0xffff; }
unsigned int Region::elementToData(unsigned short element) { return element & 15; }
