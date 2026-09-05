#include "game/world/Level.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/BoundingBox.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Explosion.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/LootTables.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ConsoleSaveFile.hpp"
#include "game/data/HierarchicalElementStorage.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/data/SavedData.hpp"
#include "game/data/SavedDataStorage.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityTickCache.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/network/LevelListener.hpp"
#include "game/network/PathNavigationListener.hpp"
#include "game/server/ObserverBlock.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelSettings.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/block/RedstoneTorchBlock.hpp"
#include "game/world/block/SnowLayerBlock.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/PlayerUID.hpp"

unsigned int TlsAlloc();
void* TlsGetValue(unsigned int index);
void TlsSetValue(unsigned int index, void* value);
void* XPhysicalAlloc(unsigned long size, unsigned long affinity, unsigned long alignment, unsigned int flags);
void XPhysicalFree(void* memory);
void XMemSet128(void* destination, int value, unsigned int size);
void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

static unsigned int sInstaTickTlsIndex = TlsAlloc();
static unsigned int sLightingCacheTlsIndex = TlsAlloc();

Level::RangeModifier::~RangeModifier()
{
}

Level::Level(boost::shared_ptr<LevelStorage> levelStorage, LevelData* levelData, Dimension* dimension,
             bool isClientSide)
{
    mSeaLevel = dimension->getSeaLevel();
    _init();
    mLevelStorage = levelStorage;
    mDimension = dimension;
    mIsClientSide = isClientSide;
    mLevelData = levelData;
    mWorldBorder = dimension->createWorldBorder(this);

    if (!mLevelData->useNewSeaLevel() && dimension->isNaturalDimension())
    {
        mSeaLevel = 64;
    }
}

Level::~Level()
{
    if (mPathListener != NULL)
    {
        delete mPathListener;
    }

    delete mRandom;

    if (mDimension != NULL)
    {
        delete mDimension;
    }

    if (mChunkSource != NULL)
    {
        delete mChunkSource;
    }

    if (mLevelData != NULL)
    {
        delete mLevelData;
    }

    delete[] mLightUpdateQueue;

    if (mWorldBorder != NULL)
    {
        delete mWorldBorder;
    }

    if (!mIsClientSide)
    {
        RedstoneTorchBlock::removeLevelReferences(this);
    }

    DeleteCriticalSection(&mLightMutex);

    if (mSavedDataStorage != NULL)
    {
        delete mSavedDataStorage;
    }

    mSavedDataStorage = NULL;

    DeleteCriticalSection(&mEntityMutex);
    DeleteCriticalSection(&mBlockEntityMutex);
}

void Level::_init()
{
    mCloudColour = 0xFFFFFF;
    mSkyDarken = 0;

    Random random;
    mRandValue = random.nextInt();
    mAddend = 1013904223;

    mPathListener = new PathNavigationListener();
    mListeners.push_back(mPathListener);

    mSkyFlashTime = 0;
    mOldRainLevel = 0.0f;
    mRainLevel = 0.0f;
    mThunderLevel = 0.0f;
    mOldThunderLevel = 0.0f;

    mRandom = new Random();
    mVillages.reset();
    mScheduledUpdatesAreImmediate = false;
    mChunkSource = NULL;
    mDimension = NULL;

    mLevelData = NULL;
    mIsClientSide = false;
    mSavedDataStorage = NULL;
    mSpawnPeacefulMobs = true;
    mSpawnHostileMobs = true;
    mFindingSpawn = false;

    InitializeCriticalSection(&mEntityMutex);
    InitializeCriticalSection(&mBlockEntityMutex);
    mProcessingBlockEntities = false;

    mLightUpdateQueue = new unsigned int[0x8000];

    InitializeCriticalSectionAndSpinCount(&mLightMutex, 0x1400);

    mRemovingBlockEntity = false;
    mAmbientTickCountdown = 0;
    mFogDistance = -1;
    mForceTimeUpdate = false;
    mLootTables = NULL;
}

bool Level::isWaterAt(const BlockPos& pos)
{
    return getBlockState(pos)->getMaterial() == Material::WATER;
}

bool Level::shouldFreeze(const BlockPos& pos, bool checkNeighbours)
{
    if (getBiome(pos)->getTemperature(pos) > 0.15f)
    {
        return false;
    }

    if ((unsigned int)pos.y >= 256)
    {
        return false;
    }

    if (getBrightness(LightLayer::BLOCK, pos) >= 10)
    {
        return false;
    }

    const BlockState* state = getBlockState(pos);
    Block* block = state->getBlock();

    if (block != Blocks::WATER && block != Blocks::FLOWING_WATER)
    {
        return false;
    }

    if (state->getValue(LiquidBlock::LEVEL) != 0)
    {
        return false;
    }

    if (checkNeighbours && isWaterAt(pos.west()) && isWaterAt(pos.east()) && isWaterAt(pos.north())
        && isWaterAt(pos.south()))
    {
        return false;
    }

    return true;
}

bool Level::shouldFreezeIgnoreNeighbors(const BlockPos& pos)
{
    if (getBiome(pos)->getTemperature(pos) > 0.15f)
    {
        return false;
    }

    if ((unsigned int)pos.y >= 256)
    {
        return false;
    }

    if (getBrightness(LightLayer::BLOCK, pos) >= 10)
    {
        return false;
    }

    const BlockState* state = getBlockState(pos);
    Block* block = state->getBlock();

    if (block != Blocks::WATER && block != Blocks::FLOWING_WATER)
    {
        return false;
    }

    return state->getValue(LiquidBlock::LEVEL) == 0;
}

long long Level::getSeed()
{
    return mLevelData->getSeed();
}

LevelData* Level::getLevelData()
{
    return mLevelData;
}

void Level::clampWithinLevelBounds(double& x, double& z, double margin)
{
    if (mDimension->getType() == DimensionType::THE_END)
    {
        const AABB* bounds = mDimension->getPlayerConstraints(BlockPos((int)x, 64, (int)z));
        x = Mth::clamp(x, bounds->mMinX + margin, bounds->mMaxX - margin);
        z = Mth::clamp(z, bounds->mMinZ + margin, bounds->mMaxZ - margin);
    }
    else
    {
        int bound = 30000000;
        LevelData* levelData = getLevelData();

        if (levelData != NULL)
        {
            bound = levelData->getXZSize() * 8;
        }

        x = Mth::clamp(x, margin - (double)bound, (double)(bound - 1) - margin);
        z = Mth::clamp(z, margin - (double)bound, (double)(bound - 1) - margin);
    }
}

bool Level::isWithinLevelBounds(int x, int z)
{
    if (mDimension->getType() == DimensionType::THE_END)
    {
        return mDimension->getPlayerConstraints(BlockPos(x, 64, z))
            ->containsIncludingLowerBound(Vec3::newTemp((double)x, 64.0, (double)z));
    }

    int bound = 30000000;
    LevelData* levelData = getLevelData();

    if (levelData != NULL)
    {
        bound = levelData->getXZSize() * 8;
    }

    return x >= -bound && z >= -bound && x < bound && z < bound;
}

void Level::clampWithinLevelHeight(double& y, double margin)
{
    if (y + margin > 512.0)
    {
        y -= (y + margin) - 512.0;
    }
}

void Level::enableLightingCache()
{
    TlsSetValue(sLightingCacheTlsIndex, XPhysicalAlloc(0x40000, 0xFFFFFFFF, 0x20000, 0x20000004));
}

void Level::destroyLightingCache()
{
    XPhysicalFree(TlsGetValue(sLightingCacheTlsIndex));
}

bool Level::getInstaTick()
{
    return TlsGetValue(sInstaTickTlsIndex) != NULL;
}

void Level::setInstaTick(bool instaTick)
{
    TlsSetValue(sInstaTickTlsIndex, (void*)(instaTick ? 1 : 0));
}

bool Level::hasEntitiesToRemove()
{
    return !mEntitiesToRemove.empty();
}

void Level::postConstruct()
{
}

LevelChunk* Level::getChunk(int chunkX, int chunkZ)
{
    return mChunkSource->getChunk(chunkX, chunkZ);
}

LevelChunk* Level::getChunkAt(const BlockPos& pos)
{
    return mChunkSource->getChunk(pos.x >> 4, pos.z >> 4);
}

bool Level::hasChunkAt(const BlockPos& pos, bool allowEmpty)
{
    return hasChunk(pos.x >> 4, pos.z >> 4, allowEmpty);
}

bool Level::hasChunkAt(const BlockPos& pos)
{
    return hasChunk(pos.x >> 4, pos.z >> 4, true);
}

Biome* Level::getBiome(const BlockPos& pos)
{
    if (hasChunkAt(pos))
    {
        return getChunkAt(pos)->getBiome(pos, mDimension->getBiomeSource());
    }

    return mDimension->getBiomeSource()->getBiome(pos, Biome::PLAINS);
}

BiomeSource* Level::getBiomeSource()
{
    return mDimension->getBiomeSource();
}

void Level::initializeLevel(LevelSettings* settings)
{
    mLevelData->setInitialized(true);
}

MinecraftServer* Level::getServer()
{
    return NULL;
}

void Level::setSpawnPos(const BlockPos& pos)
{
    mLevelData->setSpawn(pos);
}

void Level::validateSpawn()
{
    setSpawnPos(BlockPos(8, 64, 8));
}

int Level::getSeaLevel()
{
    return mSeaLevel;
}

const BlockState* Level::getTopBlockState(const BlockPos& pos)
{
    if (mLevelData->getGeneratorType() == LevelType::FLAT)
    {
        return Blocks::grass->defaultBlockState();
    }

    BlockPos top(pos.x, getSeaLevel(), pos.z);

    while (!isEmptyBlock(top.above()))
    {
        top = top.above();
    }

    return getBlockState(top);
}

bool Level::isInWorldBounds(const BlockPos& pos)
{
    return (unsigned int)pos.y < 256;
}

bool Level::isInWorldBounds(int x, int y, int z)
{
    return (unsigned int)y < 256;
}

bool Level::isOutsideBuildHeight(const BlockPos& pos)
{
    return (unsigned int)pos.y > 255;
}

bool Level::isEmptyBlock(const BlockPos& pos)
{
    return getBlockId(pos.x, pos.y, pos.z) == 0;
}

bool Level::isEmptyWaterBlock(const BlockPos& pos)
{
    return getBlockState(pos)->getMaterial() == Material::WATER;
}

bool Level::hasChunksAt(const BlockPos& pos, int radius, bool allowEmpty)
{
    return hasChunksAt(pos.x - radius, pos.y - radius, pos.z - radius, pos.x + radius, pos.y + radius,
                       pos.z + radius, allowEmpty);
}

bool Level::hasChunksAt(const BlockPos& pos, int radius)
{
    return hasChunksAt(pos.x - radius, pos.y - radius, pos.z - radius, pos.x + radius, pos.y + radius,
                       pos.z + radius, true);
}

bool Level::hasChunksAt(const BlockPos& from, const BlockPos& to, bool allowEmpty)
{
    return hasChunksAt(from.x, from.y, from.z, to.x, to.y, to.z, allowEmpty);
}

bool Level::hasChunksAt(const BlockPos& from, const BlockPos& to)
{
    return hasChunksAt(from.x, from.y, from.z, to.x, to.y, to.z, true);
}

bool Level::hasChunksAt(BoundingBox box, bool allowEmpty)
{
    return hasChunksAt(box.x0, box.y0, box.z0, box.x1, box.y1, box.z1, allowEmpty);
}

bool Level::hasChunksAt(BoundingBox box)
{
    return hasChunksAt(box, true);
}

bool Level::reallyHasChunk(int chunkX, int chunkZ)
{
    return mChunkSource->reallyHasChunk(chunkX, chunkZ);
}

bool Level::reallyHasChunkAt(const BlockPos& pos)
{
    if (!isInWorldBounds(pos))
    {
        return false;
    }

    return reallyHasChunk(pos.x >> 4, pos.z >> 4);
}

bool Level::reallyHasChunksAt(const BlockPos& from, const BlockPos& to)
{
    for (int chunkX = from.x >> 4; chunkX <= (to.x >> 4); ++chunkX)
    {
        for (int chunkZ = from.z >> 4; chunkZ <= (to.z >> 4); ++chunkZ)
        {
            if (!reallyHasChunk(chunkX, chunkZ))
            {
                return false;
            }
        }
    }

    return true;
}

bool Level::hasChunksAt(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, bool allowEmpty)
{
    if (maxY < 0 || minY > 255)
    {
        return false;
    }

    for (int chunkX = minX >> 4; chunkX <= (maxX >> 4); ++chunkX)
    {
        for (int chunkZ = minZ >> 4; chunkZ <= (maxZ >> 4); ++chunkZ)
        {
            if (!hasChunk(chunkX, chunkZ, allowEmpty))
            {
                return false;
            }
        }
    }

    return true;
}

LevelChunk* Level::getChunkAt(int x, int z)
{
    return mChunkSource->getChunk(x >> 4, z >> 4);
}

LevelChunk* Level::getChunkLoadedOrUnloadedAt(int x, int z)
{
    return mChunkSource->getChunkLoadedOrUnloaded(x >> 4, z >> 4);
}

bool Level::isChunkGeneratedAt(int chunkX, int chunkZ)
{
    if (hasChunk(chunkX, chunkZ, false))
    {
        return true;
    }

    return mChunkSource->reallyHasChunk(chunkX, chunkZ);
}

bool Level::setBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate)
{
    return internalSetBlock(false, pos, state, flags, forceUpdate);
}

bool Level::setExtraBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate)
{
    return internalSetBlock(true, pos, state, flags, forceUpdate);
}

void Level::setBlockBrightnessChanged(const BlockPos& pos)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->blockLightChanged(pos);
    }
}

void Level::setBrightness(LightLayer::variety layer, const BlockPos& pos, int brightness, bool markDirty)
{
    if (!isInWorldBounds(pos) || !hasChunkAt(pos))
    {
        return;
    }

    getChunkAt(pos)->setBrightness(layer, pos, brightness);

    if (mIsClientSide && markDirty)
    {
        if (mHasDirtyRegion)
        {
            if (pos.x < mDirtyMinX)
            {
                mDirtyMinX = pos.x;
            }

            if (pos.x > mDirtyMaxX)
            {
                mDirtyMaxX = pos.x;
            }

            if (pos.y < mDirtyMinY)
            {
                mDirtyMinY = pos.y;
            }

            if (pos.y > mDirtyMaxY)
            {
                mDirtyMaxY = pos.y;
            }

            if (pos.z < mDirtyMinZ)
            {
                mDirtyMinZ = pos.z;
            }

            if (pos.z > mDirtyMaxZ)
            {
                mDirtyMaxZ = pos.z;
            }
        }
        else
        {
            mDirtyMinX = pos.x;
            mDirtyMaxX = pos.x;
            mDirtyMinY = pos.y;
            mDirtyMaxY = pos.y;
            mDirtyMinZ = pos.z;
            mDirtyMaxZ = pos.z;
            mHasDirtyRegion = true;
        }
    }
    else
    {
        setBlockBrightnessChanged(pos);
    }
}

int Level::getBrightnessCached(unsigned int* cache, LightLayer::variety layer, int x, int y, int z)
{
    if (cache == NULL || (y & 0xFFFFFF00) != 0)
    {
        return getBrightness(layer, BlockPos(x, y, z));
    }

    int index = ((x & 0xF) << 8) | ((y & 0xF) << 4) | (z & 0xF);
    unsigned int entry = cache[index];
    unsigned int key = ((unsigned int)(y & 0xF0) << 2) | ((unsigned int)(x & 0x3F0) << 6)
                       | ((unsigned int)(z >> 4) & 0x3F);

    if ((entry & 0xFFFF) == key)
    {
        if ((entry & 0x10000000) != 0)
        {
            return (entry >> 24) & 0xF;
        }

        int brightness = getBrightness(layer, BlockPos(x, y, z));
        cache[index] = entry | ((unsigned int)brightness << 24) | 0x10000000;
        return brightness & 0xF;
    }

    if ((entry & 0x80000000) != 0)
    {
        setBrightness(layer,
                      BlockPos((int)((((entry >> 6) & 0x3F0) | (x & 0xF)) << 22) >> 22,
                               ((entry >> 2) & 0xF0) | (y & 0xF),
                               (int)((((entry & 0x3F) << 4) | (z & 0xF)) << 22) >> 22),
                      (entry >> 24) & 0xF, true);
    }

    int brightness = getBrightness(layer, BlockPos(x, y, z));
    cache[index] = key | 0x10000000 | ((unsigned int)brightness << 24);
    return brightness & 0xF;
}

bool Level::canSeeSky(const BlockPos& pos)
{
    return getChunkAt(pos)->isSkyLit(pos);
}

int Level::getEmissionCached(unsigned int* cache, Block* block, const BlockPos& pos)
{
    if (cache == NULL)
    {
        return block->defaultBlockState()->getLightEmission();
    }

    int index = ((pos.x & 0xF) << 8) | ((pos.y & 0xF) << 4) | (pos.z & 0xF);
    unsigned int entry = cache[index];
    unsigned int key = ((unsigned int)(pos.y & 0xF0) << 2) | ((unsigned int)(pos.x & 0x3F0) << 6)
                       | ((unsigned int)(pos.z >> 4) & 0x3F);

    if ((entry & 0xFFFF) == key && (entry & 0x40000000) != 0)
    {
        return (entry >> 16) & 0xF;
    }

    if ((entry & 0xFFFF) != key && (entry & 0x80000000) != 0)
    {
        setBrightness(LightLayer::BLOCK,
                      BlockPos((int)((((entry >> 6) & 0x3F0) | (pos.x & 0xF)) << 22) >> 22,
                               ((entry >> 2) & 0xF0) | (pos.y & 0xF),
                               (int)((((entry & 0x3F) << 4) | (pos.z & 0xF)) << 22) >> 22),
                      (entry >> 24) & 0xF, true);
    }

    Block* blockAt = getBlock(pos);
    Block* extraBlockAt = getExtraBlock(pos);
    int emission = blockAt->defaultBlockState()->getLightEmission();
    int blocking = blockAt->defaultBlockState()->getLightBlock();
    int extraBlocking = extraBlockAt->defaultBlockState()->getLightBlock();

    if (blocking < extraBlocking)
    {
        blocking = extraBlocking;
    }

    if ((entry & 0xFFFF) == key)
    {
        cache[index] = entry | 0x60000000 | ((unsigned int)(emission & 0xF) << 16)
                       | ((unsigned int)(blocking & 0xF) << 20);
    }
    else
    {
        cache[index] = key | 0x60000000 | ((unsigned int)(emission & 0xF) << 16)
                       | ((unsigned int)(blocking & 0xF) << 20);
    }

    return emission & 0xF;
}

int Level::getExpectedLight(unsigned int* cache, const BlockPos& pos, LightLayer::variety layer,
                            bool ignoreEmission)
{
    if (layer == LightLayer::SKY && canSeeSky(pos))
    {
        return 15;
    }

    const BlockState* state = getBlockState(pos);
    const BlockState* extraState = getExtraBlockState(pos);

    int light = (layer == LightLayer::SKY) ? 0 : state->getLightEmission();
    int blocking = state->getLightBlock();
    int extraBlocking = extraState->getLightBlock();

    if (blocking < extraBlocking)
    {
        blocking = extraBlocking;
    }

    if (blocking >= 15)
    {
        if (light < 1)
        {
            if (!ignoreEmission && layer != LightLayer::SKY)
            {
                return getEmissionCached(cache, Blocks::AIR, pos);
            }

            return 0;
        }
    }
    else if (blocking < 1)
    {
        blocking = 1;
    }

    if (light < 14)
    {
        int limit = 15 - blocking;
        int neighbour = getBrightnessCached(cache, layer, pos.x, pos.y, pos.z + 1) - blocking;

        if (light < neighbour)
        {
            light = neighbour;
        }

        if (light < limit)
        {
            neighbour = getBrightnessCached(cache, layer, pos.x, pos.y, pos.z - 1) - blocking;

            if (light < neighbour)
            {
                light = neighbour;
            }
        }

        if (light < limit)
        {
            neighbour = getBrightnessCached(cache, layer, pos.x, pos.y + 1, pos.z) - blocking;

            if (light < neighbour)
            {
                light = neighbour;
            }
        }

        if (light < limit)
        {
            neighbour = getBrightnessCached(cache, layer, pos.x + 1, pos.y, pos.z) - blocking;

            if (light < neighbour)
            {
                light = neighbour;
            }
        }

        if (light < limit)
        {
            neighbour = getBrightnessCached(cache, layer, pos.x - 1, pos.y, pos.z) - blocking;

            if (light < neighbour)
            {
                light = neighbour;
            }
        }

        if (light < limit)
        {
            neighbour = getBrightnessCached(cache, layer, pos.x, pos.y - 1, pos.z) - blocking;

            if (light < neighbour)
            {
                light = neighbour;
            }
        }
    }

    return light;
}

int Level::getBlockingCached(unsigned int* cache, LightLayer::variety layer, Block* block, Block* extraBlock,
                             const BlockPos& pos)
{
    if (cache == NULL)
    {
        if (block == NULL)
        {
            block = getBlock(pos);
        }

        if (extraBlock == NULL)
        {
            extraBlock = getExtraBlock(pos);
        }

        int blocking = block->defaultBlockState()->getLightBlock();
        int extraBlocking = extraBlock->defaultBlockState()->getLightBlock();
        return (extraBlocking < blocking) ? blocking : extraBlocking;
    }

    int index = ((pos.x & 0xF) << 8) | ((pos.y & 0xF) << 4) | (pos.z & 0xF);
    unsigned int entry = cache[index];
    unsigned int key = ((unsigned int)(pos.y & 0xF0) << 2) | ((unsigned int)(pos.x & 0x3F0) << 6)
                       | ((unsigned int)(pos.z >> 4) & 0x3F);

    if ((entry & 0xFFFF) == key && (entry & 0x40000000) != 0)
    {
        return (entry >> 20) & 0xF;
    }

    if ((entry & 0xFFFF) != key && (entry & 0x80000000) != 0)
    {
        setBrightness(layer,
                      BlockPos((int)((((entry >> 6) & 0x3F0) | (pos.x & 0xF)) << 22) >> 22,
                               ((entry >> 2) & 0xF0) | (pos.y & 0xF),
                               (int)((((entry & 0x3F) << 4) | (pos.z & 0xF)) << 22) >> 22),
                      (entry >> 24) & 0xF, true);
    }

    Block* blockAt = getBlock(pos);
    Block* extraBlockAt = getExtraBlock(pos);
    int emission = blockAt->defaultBlockState()->getLightEmission();
    int blocking = blockAt->defaultBlockState()->getLightBlock();
    int extraBlocking = extraBlockAt->defaultBlockState()->getLightBlock();

    if (blocking < extraBlocking)
    {
        blocking = extraBlocking;
    }

    if ((entry & 0xFFFF) == key)
    {
        cache[index] = entry | 0x60000000 | ((unsigned int)(emission & 0xF) << 16)
                       | ((unsigned int)(blocking & 0xF) << 20);
    }
    else
    {
        cache[index] = key | 0x60000000 | ((unsigned int)(emission & 0xF) << 16)
                       | ((unsigned int)(blocking & 0xF) << 20);
    }

    return blocking & 0xF;
}

void Level::initCachePartial(unsigned int* cache, int x, int y, int z)
{
    mHasDirtyRegion = false;

    if (cache == NULL)
    {
        return;
    }

    if ((y & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | (z & 0xF)] = 0;
        cache[(((x - 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)] = 0;
        cache[(((x + 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)] = 0;
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z - 1) & 0xF)] = 0;
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z + 1) & 0xF)] = 0;
    }

    if (((y - 1) & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | (((y - 1) << 4) & 0xF0) | (z & 0xF)] = 0;
    }

    if (((y + 1) & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | (((y + 1) << 4) & 0xF0) | (z & 0xF)] = 0;
    }
}

void Level::initCacheComplete(unsigned int* cache, int x, int y, int z)
{
    unsigned int centre = 0;
    unsigned int west = 0;
    unsigned int east = 0;
    unsigned int below = 0;
    unsigned int above = 0;
    unsigned int north = 0;
    unsigned int south = 0;

    if ((y & 0xFFFFFF00) == 0)
    {
        centre = cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | (z & 0xF)];
        west = cache[(((x - 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)];
        east = cache[(((x + 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)];
        north = cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z - 1) & 0xF)];
        south = cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z + 1) & 0xF)];
    }

    if (((y - 1) & 0xFFFFFF00) == 0)
    {
        below = cache[((x & 0xF) << 8) | (((y - 1) << 4) & 0xF0) | (z & 0xF)];
    }

    if (((y + 1) & 0xFFFFFF00) == 0)
    {
        above = cache[((x & 0xF) << 8) | (((y + 1) << 4) & 0xF0) | (z & 0xF)];
    }

    XMemSet128(cache, 0, 0x4000);

    if ((y & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | (z & 0xF)] = centre;
        cache[(((x - 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)] = west;
        cache[(((x + 1) << 8) & 0xF00) | ((y & 0xF) << 4) | (z & 0xF)] = east;
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z - 1) & 0xF)] = north;
        cache[((x & 0xF) << 8) | ((y & 0xF) << 4) | ((z + 1) & 0xF)] = south;
    }

    if (((y - 1) & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | (((y - 1) << 4) & 0xF0) | (z & 0xF)] = below;
    }

    if (((y + 1) & 0xFFFFFF00) == 0)
    {
        cache[((x & 0xF) << 8) | (((y + 1) << 4) & 0xF0) | (z & 0xF)] = above;
    }
}

void Level::setBrightnessCached(unsigned int* cache, unsigned long long* dirty, LightLayer::variety layer,
                                int x, int y, int z, int brightness)
{
    if (cache == NULL)
    {
        setBrightness(layer, BlockPos(x, y, z), brightness, true);
        return;
    }

    if ((y & 0xFFFFFF00) != 0)
    {
        return;
    }

    int index = ((x & 0xF) << 8) | ((y & 0xF) << 4) | (z & 0xF);
    unsigned int entry = cache[index];
    unsigned int key = ((unsigned int)(y & 0xF0) << 2) | ((unsigned int)(x & 0x3F0) << 6)
                       | ((unsigned int)(z >> 4) & 0x3F);

    if ((entry & 0xFFFF) != key)
    {
        if ((entry & 0x80000000) != 0)
        {
            setBrightness(layer,
                          BlockPos((int)((((entry >> 6) & 0x3F0) | (x & 0xF)) << 22) >> 22,
                                   ((entry >> 2) & 0xF0) | (y & 0xF),
                                   (int)((((entry & 0x3F) << 4) | (z & 0xF)) << 22) >> 22),
                          (entry >> 24) & 0xF, true);
        }

        entry = key;
    }

    *dirty |= (1ULL << (x & 0xF)) | (0x10000ULL << (y & 0xF)) | (1ULL << (32 + (z & 0xF)));
    cache[index] = (entry & 0xF0FFFFFF) | ((unsigned int)brightness << 24) | 0x90000000;
}

void Level::setBlocksDirty(const BlockPos& from, const BlockPos& to)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->setBlocksDirty(from.x, from.y, from.z, to.x, to.y, to.z, this);
    }
}

void Level::flushCache(unsigned int* cache, unsigned long long dirty, LightLayer::variety layer)
{
    if (dirty == 0)
    {
        return;
    }

    if (cache != NULL)
    {
        unsigned int* entryPtr = cache;

        for (int x = 0; x < 16; ++x)
        {
            if ((dirty & (1ULL << x)) == 0)
            {
                entryPtr += 256;
                continue;
            }

            for (int y = 0; y < 16; ++y)
            {
                if ((dirty & (0x10000ULL << y)) == 0)
                {
                    entryPtr += 16;
                    continue;
                }

                for (int z = 0; z < 16; ++z, ++entryPtr)
                {
                    if ((dirty & (1ULL << (32 + z))) == 0)
                    {
                        continue;
                    }

                    unsigned int entry = *entryPtr;

                    if ((entry & 0x80000000) != 0)
                    {
                        setBrightness(layer,
                                      BlockPos((int)((((entry >> 6) & 0x3F0) | x) << 22) >> 22,
                                               ((entry >> 2) & 0xF0) | (y & 0xF),
                                               (int)(((z & 0xF) | ((entry & 0x3F) << 4)) << 22) >> 22),
                                      (entry >> 24) & 0xF, true);
                    }
                }
            }
        }
    }

    if (mIsClientSide && mHasDirtyRegion)
    {
        setBlocksDirty(BlockPos(mDirtyMinX, mDirtyMinY, mDirtyMinZ),
                       BlockPos(mDirtyMaxX, mDirtyMaxY, mDirtyMaxZ));
    }
}

bool Level::checkLight(LightLayer::variety layer, const BlockPos& pos, bool onlyLoadedChunks, bool skipCentre)
{
    unsigned int* cache = (unsigned int*)TlsGetValue(sLightingCacheTlsIndex);
    unsigned long long dirty = 0;

    if (!hasChunksAt(pos, onlyLoadedChunks ? 0 : 17))
    {
        return false;
    }

    EnterCriticalSection(&mLightMutex);

    int centreX = pos.x;
    int centreY = pos.y;
    int centreZ = pos.z;

    PIXBeginNamedEvent(0.0f, "initCachePartial");
    initCachePartial(cache, centreX, centreY, centreZ);
    PIXEndNamedEvent();

    unsigned int* queue = (cache == NULL) ? mLightUpdateQueue : cache + 0x1000;
    int minBound = (mDimension->getXZSize() * -16) >> 1;
    int maxBound = ((mDimension->getXZSize() << 4) >> 1) - 1;

    if (centreX > maxBound || centreX < minBound || centreZ > maxBound || centreZ < minBound)
    {
        LeaveCriticalSection(&mLightMutex);
        return false;
    }

    int toCheck = 0;
    int checked = 0;

    PIXBeginNamedEvent(0.0f, "First block");

    int currentLight = getBrightnessCached(cache, layer, pos.x, pos.y, pos.z);
    int expectedLight = getExpectedLight(cache, pos, layer, false);

    if (expectedLight != currentLight && cache != NULL)
    {
        initCacheComplete(cache, centreX, centreY, centreZ);
    }

    if (expectedLight < currentLight)
    {
        queue[0] = ((unsigned int)currentLight << 18) | 0x20820;

        int count = 1;
        MutableBlockPos current(0, 0, 0);
        unsigned int* read = queue;
        unsigned int* keep = queue;

        do
        {
            unsigned int entry = *read;
            ++read;
            ++checked;

            int light = (entry >> 18) & 0xF;
            int x = (int)(entry & 0x3F) - 32 + centreX;
            int y = (int)((entry >> 6) & 0x3F) - 32 + centreY;
            int z = (int)((entry >> 12) & 0x3F) - 32 + centreZ;
            current.set(x, y, z);

            if (getBrightnessCached(cache, layer, x, y, z) != light)
            {
                continue;
            }

            setBrightnessCached(cache, &dirty, layer, x, y, z, 0);

            if (light == 0)
            {
                continue;
            }

            if (Mth::abs(x - centreX) + Mth::abs(y - centreY) + Mth::abs(z - centreZ) >= 17)
            {
                continue;
            }

            bool stillLit = false;

            for (int i = 0; i < 6; ++i)
            {
                const Direction* direction = Direction::from3DDataValue(i);
                int nx = x + direction->getStepX();
                int ny = y + direction->getStepY();
                int nz = z + direction->getStepZ();

                if (nx > maxBound || nx < minBound || nz > maxBound || nz < minBound
                    || (unsigned int)ny >= 256)
                {
                    continue;
                }

                current.set(nx, ny, nz);

                int emission = 0;
                int blocking;

                if (layer == LightLayer::SKY)
                {
                    blocking = getBlockingCached(cache, LightLayer::SKY, getBlock(current),
                                                 getExtraBlock(current), current);
                }
                else
                {
                    emission = getBlock(current)->defaultBlockState()->getLightEmission();
                    blocking = getBlockingCached(cache, layer, getBlock(current), getExtraBlock(current),
                                                 current);
                }

                if (blocking < 1)
                {
                    blocking = 1;
                }

                if (blocking > 14 && emission > 0)
                {
                    blocking = 1;
                }

                int neighbourLight = getBrightnessCached(cache, layer, nx, ny, nz);
                int fallOff = light - blocking;

                if (fallOff < neighbourLight)
                {
                    stillLit = true;
                }
                else if (fallOff == neighbourLight && count < 0x8000)
                {
                    queue[count] = (unsigned int)(nx - centreX + 32)
                                   | ((unsigned int)(ny - centreY + 32) << 6)
                                   | ((unsigned int)(nz - centreZ + 32) << 12)
                                   | ((unsigned int)fallOff << 18);
                    ++count;
                }
            }

            if (stillLit)
            {
                *keep = entry;
                ++keep;
                ++toCheck;
            }
        }
        while (checked < count);
    }
    else if (expectedLight > currentLight)
    {
        queue[0] = 0x20820;
        toCheck = 1;
    }

    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "Second block: checked=%i toCheck=%i", 0, toCheck);

    MutableBlockPos current(0, 0, 0);
    checked = 0;

    while (checked < toCheck)
    {
        unsigned int entry = queue[checked];
        ++checked;

        int x = (int)(entry & 0x3F) - 32 + centreX;
        int y = (int)((entry >> 6) & 0x3F) - 32 + centreY;
        int z = (int)((entry >> 12) & 0x3F) - 32 + centreZ;
        current.set(x, y, z);

        if (onlyLoadedChunks && !hasChunkAt(current))
        {
            continue;
        }

        int light = getBrightnessCached(cache, layer, x, y, z);
        bool ignoreEmission = layer == LightLayer::BLOCK && skipCentre
                              && !(x == centreX && y == centreY && z == centreZ);
        int expected = getExpectedLight(cache, current, layer, ignoreEmission);

        if (expected == light)
        {
            continue;
        }

        setBrightnessCached(cache, &dirty, layer, x, y, z, expected);

        if (light >= expected)
        {
            continue;
        }

        if (Mth::abs(x - centreX) + Mth::abs(y - centreY) + Mth::abs(z - centreZ) >= 17
            || toCheck >= 0x7FFA)
        {
            continue;
        }

        if (x - 1 >= minBound && getBrightnessCached(cache, layer, x - 1, y, z) < expected)
        {
            queue[toCheck] = (unsigned int)(x - 1 - centreX + 32)
                             | ((unsigned int)(y - centreY + 32) << 6)
                             | ((unsigned int)(z - centreZ + 32) << 12);
            ++toCheck;
        }

        if (x + 1 <= maxBound && getBrightnessCached(cache, layer, x + 1, y, z) < expected)
        {
            queue[toCheck] = (unsigned int)(x + 1 - centreX + 32)
                             | ((unsigned int)(y - centreY + 32) << 6)
                             | ((unsigned int)(z - centreZ + 32) << 12);
            ++toCheck;
        }

        if (y - 1 >= 0 && getBrightnessCached(cache, layer, x, y - 1, z) < expected)
        {
            queue[toCheck] = (unsigned int)(x - centreX + 32)
                             | ((unsigned int)(y - 1 - centreY + 32) << 6)
                             | ((unsigned int)(z - centreZ + 32) << 12);
            ++toCheck;
        }

        if (y + 1 < 256 && getBrightnessCached(cache, layer, x, y + 1, z) < expected)
        {
            queue[toCheck] = (unsigned int)(x - centreX + 32)
                             | ((unsigned int)(y + 1 - centreY + 32) << 6)
                             | ((unsigned int)(z - centreZ + 32) << 12);
            ++toCheck;
        }

        if (z - 1 >= minBound && getBrightnessCached(cache, layer, x, y, z - 1) < expected)
        {
            queue[toCheck] = (unsigned int)(x - centreX + 32)
                             | ((unsigned int)(y - centreY + 32) << 6)
                             | ((unsigned int)(z - 1 - centreZ + 32) << 12);
            ++toCheck;
        }

        if (z + 1 <= maxBound && getBrightnessCached(cache, layer, x, y, z + 1) < expected)
        {
            queue[toCheck] = (unsigned int)(x - centreX + 32)
                             | ((unsigned int)(y - centreY + 32) << 6)
                             | ((unsigned int)(z + 1 - centreZ + 32) << 12);
            ++toCheck;
        }
    }

    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "flushCache");
    flushCache(cache, dirty, layer);
    PIXEndNamedEvent();

    LeaveCriticalSection(&mLightMutex);
    return true;
}

bool Level::checkLight(const BlockPos& pos, bool onlyLoadedChunks, bool skipCentre)
{
    bool changed = false;

    if (mDimension->isHasSkyLight())
    {
        PIXBeginNamedEvent(0.0f, "Check sky light");
        changed = checkLight(LightLayer::SKY, pos, onlyLoadedChunks, false);
        PIXEndNamedEvent();
    }

    PIXBeginNamedEvent(0.0f, "Check block light");
    bool blockChanged = checkLight(LightLayer::BLOCK, pos, onlyLoadedChunks, skipCentre);
    PIXEndNamedEvent();

    return changed || blockChanged;
}

void Level::neighborChanged(const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    if (mIsClientSide)
    {
        return;
    }

    getBlockState(pos)->neighborChanged(this, pos, block, fromPos);
    Block::getStateByElementId(getExtraElement(pos.x, pos.y, pos.z))
        ->neighborChanged(this, pos, block, fromPos);
}

void Level::observedNeighborChanged(const BlockPos& pos, Block* block, const BlockPos& fromPos)
{
    if (mIsClientSide)
    {
        return;
    }

    const BlockState* state = getBlockState(pos);

    if (state->getBlock() == Blocks::observer)
    {
        ObserverBlock::observedNeighborChanged(state, this, pos, block, fromPos);
    }
}

void Level::updateObservingBlocksAt(const BlockPos& pos, Block* block)
{
    observedNeighborChanged(pos.west(), block, pos);
    observedNeighborChanged(pos.east(), block, pos);
    observedNeighborChanged(pos.below(), block, pos);
    observedNeighborChanged(pos.above(), block, pos);
    observedNeighborChanged(pos.north(), block, pos);
    observedNeighborChanged(pos.south(), block, pos);
}

void Level::updateNeighborsAt(const BlockPos& pos, Block* block, bool updateObservers)
{
    neighborChanged(pos.west(), block, pos);
    neighborChanged(pos.east(), block, pos);
    neighborChanged(pos.below(), block, pos);
    neighborChanged(pos.above(), block, pos);
    neighborChanged(pos.north(), block, pos);
    neighborChanged(pos.south(), block, pos);

    if (updateObservers)
    {
        updateObservingBlocksAt(pos, block);
    }
}

void Level::blockUpdated(const BlockPos& pos, Block* block, bool updateObservers)
{
    updateNeighborsAt(pos, block, updateObservers);
}

bool Level::internalSetBlock(bool extraBlock, const BlockPos& pos, const BlockState* state, int flags,
                             bool forceUpdate)
{
    if (isOutsideBuildHeight(pos))
    {
        return false;
    }

    LevelChunk* chunk = getChunkAt(pos);
    Block* block = state->getBlock();

    int oldBlocking = chunk->getBlock(pos)->defaultBlockState()->getLightBlock();
    int oldExtraBlocking = chunk->getExtraBlock(pos)->defaultBlockState()->getLightBlock();

    if (oldBlocking < oldExtraBlocking)
    {
        oldBlocking = oldExtraBlocking;
    }

    const BlockState* oldState;

    if (!extraBlock)
    {
        oldState = chunk->setBlock(pos, state);
    }
    else
    {
        oldState = chunk->setExtraBlock(pos, state);
    }

    if (oldState == NULL)
    {
        return false;
    }

    bool sameLight = false;
    int newBlocking = chunk->getBlock(pos)->defaultBlockState()->getLightBlock();
    int newExtraBlocking = chunk->getExtraBlock(pos)->defaultBlockState()->getLightBlock();

    if (newBlocking < newExtraBlocking)
    {
        newBlocking = newExtraBlocking;
    }

    if (oldBlocking == newBlocking && state->getLightEmission() == oldState->getLightEmission())
    {
        sameLight = true;
    }

    if ((flags & 32) == 0 || !sameLight)
    {
        PIXBeginNamedEvent(0.0f, "Checking light: %d, %d, %d", pos.x, pos.y, pos.z);
        checkLight(pos, false, false);
        PIXEndNamedEvent();
    }

    bool sendUpdate = forceUpdate;

    if (!sendUpdate && (flags & 2) != 0)
    {
        if (mIsClientSide)
        {
            if ((flags & 4) != 0)
            {
                return true;
            }

            sendUpdate = true;
        }
        else if (chunk->isReady())
        {
            sendUpdate = true;
        }
    }

    if (sendUpdate)
    {
        sendBlockUpdated(pos, oldState, state, flags, true);
    }

    if (mIsClientSide)
    {
        return true;
    }

    if (!forceUpdate && (flags & 1) == 0)
    {
        if ((flags & 0x10) == 0)
        {
            updateObservingBlocksAt(pos, block);
        }

        return true;
    }

    blockUpdated(pos, oldState->getBlock(), ((flags >> 4) & 1) == 0);

    if (state->hasAnalogOutputSignal())
    {
        updateNeighbourForOutputSignal(pos, block);
    }

    if (extraBlock && state != oldState)
    {
        neighborChanged(pos, block, pos);
    }

    return true;
}

bool Level::setData(const BlockPos& pos, int data, int flags, bool forceUpdate)
{
    return setData(pos.x, pos.y, pos.z, data, flags, forceUpdate);
}

bool Level::setData(int x, int y, int z, int data, int flags, bool forceUpdate)
{
    if (!isInWorldBounds(x, y, z))
    {
        return false;
    }

    LevelChunk* chunk = getChunk(x >> 4, z >> 4);
    BlockPos local(x & 0xF, y, z & 0xF);
    bool lightChanged;
    bool changed = chunk->setData(x & 0xF, y, z & 0xF, data, getBlock(local)->mBlockData, &lightChanged);

    if (!changed && !forceUpdate)
    {
        return changed;
    }

    Block* block = chunk->getBlock(local);
    BlockPos pos(x, y, z);

    if (forceUpdate || (flags & 2) != 0)
    {
        if (forceUpdate || !mIsClientSide || (flags & 4) == 0)
        {
            sendBlockUpdated(pos, block->defaultBlockState(), block->defaultBlockState(), flags, true);
        }
        else
        {
            return changed;
        }
    }

    if (mIsClientSide)
    {
        return changed;
    }

    if (!forceUpdate && (flags & 1) == 0)
    {
        if ((flags & 0x10) == 0)
        {
            updateObservingBlocksAt(pos, block);
        }
    }
    else
    {
        blockUpdated(pos, block, false);

        if (block != NULL && block->getBlockState(data)->hasAnalogOutputSignal())
        {
            updateNeighbourForOutputSignal(pos, block);
        }
    }

    return changed;
}

bool Level::setBlockAndData(int x, int y, int z, int blockId, int data, int flags)
{
    if (!isInWorldBounds(x, y, z))
    {
        return false;
    }

    LevelChunk* chunk = getChunk(x >> 4, z >> 4);

    if ((flags & 0x40) != 0)
    {
        chunk->setBlockAndData(x & 0xF, y, z & 0xF, blockId, data, true, false);
        return true;
    }

    int oldBlockId = chunk->getBlockId(x & 0xF, y, z & 0xF);
    bool changed = chunk->setBlockAndData(x & 0xF, y, z & 0xF, blockId, data, true, false);
    BlockPos pos(x, y, z);

    if ((flags & 0x20) == 0)
    {
        checkLight(pos, false, false);
    }

    if (!changed)
    {
        return changed;
    }

    Block* block = Block::byId(blockId);
    Block* oldBlock = Block::byId(oldBlockId);
    bool immediate = false;

    if ((flags & 2) != 0 && (!mIsClientSide || (flags & 4) == 0))
    {
        immediate = true;
    }

    sendBlockUpdated(pos, block->defaultBlockState(), oldBlock->defaultBlockState(), flags, immediate);

    if (mIsClientSide)
    {
        return changed;
    }

    if ((flags & 1) == 0)
    {
        if ((flags & 0x10) == 0)
        {
            updateObservingBlocksAt(pos, block);
        }
    }
    else
    {
        blockUpdated(pos, oldBlock, ((flags >> 4) & 1) == 0);

        if (block != NULL && block->getBlockState(data)->hasAnalogOutputSignal())
        {
            updateNeighbourForOutputSignal(pos, block);
        }
    }

    return changed;
}

bool Level::removeBlock(const BlockPos& pos)
{
    return setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
}

void Level::levelEvent(boost::shared_ptr<Player> player, int type, const BlockPos& pos, int data)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->levelEvent(player, type, pos, data);
    }
}

void Level::levelEvent(int type, const BlockPos& pos, int data)
{
    levelEvent(boost::shared_ptr<Player>(), type, pos, data);
}

bool Level::destroyBlock(const BlockPos& pos, bool dropResources)
{
    const BlockState* state = getBlockState(pos);
    Block* block = state->getBlock();

    if (state->getMaterial() == Material::AIR)
    {
        return false;
    }

    levelEvent(2001, pos, Block::getIdWithData(state));

    if (dropResources)
    {
        block->spawnResources(this, pos, state, 0);
    }

    return setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
}

bool Level::setBlockAndUpdate(const BlockPos& pos, const BlockState* state)
{
    return setBlock(pos, state, 3, false);
}

void Level::sendBlockUpdated(const BlockPos& pos, const BlockState* oldState, const BlockState* newState,
                             int flags, bool immediate)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->blockChanged(this, pos, oldState, newState, flags, immediate);
    }
}

void Level::lightColumnChanged(int x, int z, int fromY, int toY)
{
    PIXBeginNamedEvent(0.0f, "LightColumnChanged (%d, %d) %d to %d", x, z, fromY, toY);

    int minY = fromY;
    int maxY = toY;

    if (toY < fromY)
    {
        minY = toY;
        maxY = fromY;
    }

    if (mDimension->isHasSkyLight())
    {
        PIXBeginNamedEvent(0.0f, "Checking lights");

        for (int y = minY; y <= maxY; ++y)
        {
            PIXBeginNamedEvent(0.0f, "Checking light %d", y);
            checkLight(LightLayer::SKY, BlockPos(x, y, z), false, false);
            PIXEndNamedEvent();
        }

        PIXEndNamedEvent();
    }

    PIXBeginNamedEvent(0.0f, "Setting blocks dirty");
    setBlocksDirty(BlockPos(x, minY, z), BlockPos(x, maxY, z));
    PIXEndNamedEvent();

    PIXEndNamedEvent();
}

void Level::updateNeighborsAtExceptFromFacing(const BlockPos& pos, Block* block, const Direction* skipSide)
{
    if (skipSide != Direction::WEST)
    {
        neighborChanged(pos.west(), block, pos);
    }

    if (skipSide != Direction::EAST)
    {
        neighborChanged(pos.east(), block, pos);
    }

    if (skipSide != Direction::DOWN)
    {
        neighborChanged(pos.below(), block, pos);
    }

    if (skipSide != Direction::UP)
    {
        neighborChanged(pos.above(), block, pos);
    }

    if (skipSide != Direction::NORTH)
    {
        neighborChanged(pos.north(), block, pos);
    }

    if (skipSide != Direction::SOUTH)
    {
        neighborChanged(pos.south(), block, pos);
    }
}

bool Level::isBlockToBeTickedAt(const BlockPos& pos, Block* block)
{
    return false;
}

bool Level::canSeeSkyFromBelowWater(const BlockPos& pos)
{
    if (pos.y > 62)
    {
        return canSeeSky(pos);
    }

    BlockPos current(pos.x, 63, pos.z);

    if (!canSeeSky(current))
    {
        return false;
    }

    for (current = current.below(); current.y > pos.y; current = current.below())
    {
        const BlockState* state = getBlockState(current);
        const BlockState* extraState = getExtraBlockState(current);
        int blocking = state->getLightBlock();
        int extraBlocking = extraState->getLightBlock();

        if (blocking < extraBlocking)
        {
            blocking = extraBlocking;
        }

        if (blocking > 0 && !state->getMaterial()->isLiquid() && !extraState->getMaterial()->isLiquid())
        {
            return false;
        }
    }

    return true;
}

int Level::getDaytimeRawBrightness(const BlockPos& pos)
{
    if (pos.y < 0)
    {
        return 0;
    }

    BlockPos clamped = pos;

    if (pos.y > 255)
    {
        clamped = BlockPos(pos.x, 255, pos.z);
    }

    return getChunkAt(clamped)->getRawBrightness(clamped, 0);
}

int Level::getRawBrightness(const BlockPos& pos, bool useNeighbourBrightness)
{
    if (!isWithinLevelBounds(pos.x, pos.z))
    {
        return 15;
    }

    if (useNeighbourBrightness && getBlockState(pos)->doesPropagate())
    {
        int brightest = getRawBrightness(pos.above(), false);
        int east = getRawBrightness(pos.east(), false);
        int west = getRawBrightness(pos.west(), false);
        int south = getRawBrightness(pos.south(), false);
        int north = getRawBrightness(pos.north(), false);

        if (brightest < east)
        {
            brightest = east;
        }

        if (brightest < west)
        {
            brightest = west;
        }

        if (brightest < south)
        {
            brightest = south;
        }

        if (brightest < north)
        {
            brightest = north;
        }

        return brightest;
    }

    if (pos.y < 0)
    {
        return 0;
    }

    BlockPos clamped = pos;

    if (pos.y > 255)
    {
        clamped = BlockPos(pos.x, 255, pos.z);
    }

    return getChunkAt(clamped)->getRawBrightness(clamped, mSkyDarken);
}

int Level::getRawBrightness(const BlockPos& pos)
{
    if (!isWithinLevelBounds(pos.x, pos.z))
    {
        return 15;
    }

    if (getBlockState(pos)->doesPropagate())
    {
        int brightest = getRawBrightness(pos.above(), false);
        int east = getRawBrightness(pos.east(), false);
        int west = getRawBrightness(pos.west(), false);
        int south = getRawBrightness(pos.south(), false);
        int north = getRawBrightness(pos.north(), false);

        if (brightest < east)
        {
            brightest = east;
        }

        if (brightest < west)
        {
            brightest = west;
        }

        if (brightest < south)
        {
            brightest = south;
        }

        if (brightest < north)
        {
            brightest = north;
        }

        return brightest;
    }

    if (pos.y < 0)
    {
        return 0;
    }

    BlockPos clamped = pos;

    if (pos.y > 255)
    {
        clamped = BlockPos(pos.x, 255, pos.z);
    }

    return getChunkAt(clamped)->getRawBrightness(clamped, mSkyDarken);
}

int Level::getHeightmapHeight(int x, int z)
{
    if (!isWithinLevelBounds(x, z))
    {
        return getSeaLevel() + 1;
    }

    if (!hasChunk(x >> 4, z >> 4, true))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getHeightmap(x & 0xF, z & 0xF);
}

BlockPos Level::getHeightmapPos(const BlockPos& pos)
{
    return BlockPos(pos.x, getHeightmapHeight(pos.x, pos.z), pos.z);
}

int Level::getHeightmap(int x, int z)
{
    if (!isWithinLevelBounds(x, z))
    {
        return 64;
    }

    if (!hasChunk(x >> 4, z >> 4, true))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getHeightmap(x & 0xF, z & 0xF);
}

int Level::getLowestHeightmap(int x, int z)
{
    if (!isWithinLevelBounds(x, z))
    {
        return 64;
    }

    if (!hasChunk(x >> 4, z >> 4, true))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getLowestHeightmap();
}

int Level::getBrightnessPropagate(LightLayer::variety layer, const BlockPos& pos)
{
    if (!mDimension->isHasSkyLight() && layer == LightLayer::SKY)
    {
        return 0;
    }

    BlockPos target = pos;

    if (pos.y < 0)
    {
        target = BlockPos(pos.x, 0, pos.z);
    }

    if (!isInWorldBounds(target) || !hasChunkAt(target))
    {
        return layer;
    }

    if (!getBlockState(target)->doesPropagate())
    {
        return getChunkAt(target)->getBrightness(layer, target);
    }

    int brightest = getBrightness(layer, target.above());
    int east = getBrightness(layer, target.east());
    int west = getBrightness(layer, target.west());
    int south = getBrightness(layer, target.south());
    int north = getBrightness(layer, target.north());

    if (brightest < east)
    {
        brightest = east;
    }

    if (brightest < west)
    {
        brightest = west;
    }

    if (brightest < south)
    {
        brightest = south;
    }

    if (brightest < north)
    {
        brightest = north;
    }

    return brightest;
}

void Level::getNeighbourBrightnesses(int* brightnesses, LightLayer::variety layer, int x, int y, int z)
{
    if ((x & 0xF) != 0 && (x & 0xF) != 15 && (z & 0xF) != 0 && (z & 0xF) != 15
        && (unsigned int)(y - 1) < 126)
    {
        int size = mChunkCacheSize;
        int half = size / 2;
        int cacheX = (x >> 4) + half;
        int cacheZ = (z >> 4) + half;

        if (cacheX >= 0 && cacheX < size && cacheZ >= 0 && cacheZ < size)
        {
            LevelChunk* chunk = mChunkCache[cacheX * size + cacheZ];

            if (chunk != NULL)
            {
                chunk->getNeighbourBrightnesses(brightnesses, layer, x & 0xF, y, z & 0xF);
                return;
            }
        }

        brightnesses[0] = layer;
        brightnesses[1] = layer;
        brightnesses[2] = layer;
        brightnesses[3] = layer;
        brightnesses[4] = layer;
        brightnesses[5] = layer;
        return;
    }

    brightnesses[0] = getBrightness(layer, BlockPos(x - 1, y, z));
    brightnesses[1] = getBrightness(layer, BlockPos(x + 1, y, z));
    brightnesses[2] = getBrightness(layer, BlockPos(x, y - 1, z));
    brightnesses[3] = getBrightness(layer, BlockPos(x, y + 1, z));
    brightnesses[4] = getBrightness(layer, BlockPos(x, y, z - 1));
    brightnesses[5] = getBrightness(layer, BlockPos(x, y, z + 1));
}

int Level::getBrightness(LightLayer::variety layer, const BlockPos& pos)
{
    int size = mChunkCacheSize;
    int half = size / 2;
    int cacheX = (pos.x >> 4) + half;
    int cacheZ = (pos.z >> 4) + half;

    if (cacheX < 0 || cacheX >= size || cacheZ < 0 || cacheZ >= size)
    {
        return 0;
    }

    LevelChunk* chunk = mChunkCache[cacheX * size + cacheZ];

    if (chunk != NULL)
    {
        BlockPos target(pos.x, pos.y, pos.z);

        if (target.y < 0)
        {
            target = BlockPos(target.x, 0, target.z);
        }

        if (isInWorldBounds(target))
        {
            return chunk->getBrightness(layer, target);
        }
    }

    return layer;
}

int Level::getLightColor(const BlockPos& pos, int lightValue, int cacheIndex)
{
    int skyLight = getBrightnessPropagate(LightLayer::SKY, pos);
    int blockLight = getBrightnessPropagate(LightLayer::BLOCK, pos);

    if (blockLight < lightValue)
    {
        blockLight = lightValue;
    }

    return (skyLight << 20) | (blockLight << 4);
}

float Level::getBrightness(const BlockPos& pos, int lightValue)
{
    int brightness = getRawBrightness(pos);

    if (brightness < lightValue)
    {
        brightness = lightValue;
    }

    return mDimension->getBrightnessRamp()[brightness];
}

float Level::getBrightness(const BlockPos& pos)
{
    return mDimension->getBrightnessRamp()[getRawBrightness(pos)];
}

const BlockState* Level::getBlockState(const BlockPos& pos)
{
    if (isOutsideBuildHeight(pos))
    {
        return Blocks::AIR->defaultBlockState();
    }

    return getChunkAt(pos)->getBlockState(pos);
}

const BlockState* Level::getBlockState(EntityTickCache& tickCache, const BlockPos& pos)
{
    const BlockState* state = tickCache.getBlockState(pos.x, pos.y, pos.z);

    if (state == NULL)
    {
        return getBlockState(pos);
    }

    return state;
}

const BlockState* Level::getBlockState(LevelChunk* chunk, EntityTickCache& tickCache, const BlockPos& pos)
{
    const BlockState* state = tickCache.getBlockState(pos.x, pos.y, pos.z);

    if (state != NULL)
    {
        return state;
    }

    if (isOutsideBuildHeight(pos))
    {
        return Blocks::AIR->defaultBlockState();
    }

    return chunk->getBlockState(pos);
}

int Level::getBlockId(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getBlockId(x & 0xF, y, z & 0xF);
}

int Level::getBlockId(EntityTickCache& tickCache, int x, int y, int z)
{
    int element = tickCache.getElement(x, y, z);

    if (element == 0xFFFF)
    {
        element = getElement(x, y, z);
    }

    return element >> 4;
}

unsigned short Level::getElement(EntityTickCache& tickCache, int x, int y, int z)
{
    unsigned short element = tickCache.getElement(x, y, z);

    if (element == 0xFFFF)
    {
        return getElement(x, y, z);
    }

    return element;
}

unsigned short Level::getElement(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getElement(x & 0xF, y, z & 0xF);
}

unsigned short Level::getExtraElement(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getExtraElement(x & 0xF, y, z & 0xF);
}

void Level::getElementAndExtraElement(int x, int y, int z, unsigned short* element,
                                      unsigned short* extraElement)
{
    if (!isInWorldBounds(x, y, z))
    {
        *element = 0;
        *extraElement = 0;
        return;
    }

    getChunk(x >> 4, z >> 4)->getElementAndExtraElement(x & 0xF, y, z & 0xF, element, extraElement);
}

bool Level::sectionContainsBlockId(int x, int y, int z, int blockId)
{
    if (!isInWorldBounds(x, y, z))
    {
        return false;
    }

    return getChunk(x >> 4, z >> 4)->sectionContainsBlockId(x & 0xF, y, z & 0xF, blockId);
}

int Level::getExtraBlockId(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getExtraBlockId(x & 0xF, y, z & 0xF);
}

void Level::getBlockAndExtraBlockId(int x, int y, int z, int* blockId, int* extraBlockId)
{
    if (!isInWorldBounds(x, y, z))
    {
        *blockId = 0;
        *extraBlockId = 0;
        return;
    }

    getChunk(x >> 4, z >> 4)->getBlockAndExtraId(x & 0xF, y, z & 0xF, blockId, extraBlockId);
}

Block* Level::getExtraBlock(const BlockPos& pos)
{
    return Block::byId(getExtraBlockId(pos.x, pos.y, pos.z));
}

void Level::getBlockAndExtraBlock(const BlockPos& pos, Block** block, Block** extraBlock)
{
    int blockId = 0;
    int extraBlockId = 0;
    getBlockAndExtraBlockId(pos.x, pos.y, pos.z, &blockId, &extraBlockId);
    *block = Block::byId(blockId);
    *extraBlock = Block::byId(extraBlockId);
}

Block* Level::getLiquidBlock(const BlockPos& pos)
{
    Block* extraBlock = getExtraBlock(pos);

    if (extraBlock == NULL || extraBlock == Blocks::AIR)
    {
        return getBlock(pos);
    }

    return extraBlock;
}

const BlockState* Level::getLiquidBlockState(const BlockPos& pos)
{
    const BlockState* extraState = getExtraBlockState(pos);

    if (extraState != NULL && extraState != Blocks::AIR->defaultBlockState())
    {
        return extraState;
    }

    return getBlockState(pos);
}

const BlockState* Level::getExtraBlockState(const BlockPos& pos)
{
    if (isOutsideBuildHeight(pos))
    {
        return Blocks::AIR->defaultBlockState();
    }

    return getChunkAt(pos)->getExtraBlockState(pos);
}

int Level::getBlockData(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getData(x & 0xF, y, z & 0xF);
}

int Level::getExtraBlockData(int x, int y, int z)
{
    if (!isInWorldBounds(x, y, z))
    {
        return 0;
    }

    return getChunk(x >> 4, z >> 4)->getExtraBlockData(x & 0xF, y, z & 0xF);
}

Block* Level::getBlock(const BlockPos& pos)
{
    return Block::byId(getBlockId(pos.x, pos.y, pos.z));
}

bool Level::isSolidRender(const BlockPos& pos)
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
        const BlockState* neighbour = getBlockState(pos.offset(xOffsets[i], yOffsets[i], zOffsets[i]));

        if (!LeafBlock::isLeaves(neighbour->getBlock()) || !neighbour->isSolidRender())
        {
            return false;
        }
    }

    return true;
}

bool Level::isDay()
{
    return mSkyDarken < 4;
}

HitResult* Level::clip(Vec3* from, Vec3* to, bool stopOnLiquid, bool ignoreBlocksWithoutShape,
                       bool returnLastUncollidableBlock, bool testUseBlocks,
                       boost::unordered_set<Block*, boost::hash<Block*>, std::equal_to<Block*>,
                                            std::allocator<Block*> >* alwaysClipBlocks,
                       bool clipExtraBlocks)
{
    int targetX = Mth::fastFloor(to->x);
    int targetY = Mth::fastFloor(to->y);
    int targetZ = Mth::fastFloor(to->z);
    int currentX = Mth::fastFloor(from->x);
    int currentY = Mth::fastFloor(from->y);
    int currentZ = Mth::fastFloor(from->z);

    MutableBlockPos current(currentX, currentY, currentZ);
    MutableBlockPos target(targetX, targetY, targetZ);

    const BlockState* state = getBlockState(current);
    const BlockState* extraState = getExtraBlockState(current);
    Block* block = state->getBlock();
    Block* extraBlock = extraState->getBlock();

    if (!ignoreBlocksWithoutShape || state->getClipAABB(this, current) != NULL
        || (testUseBlocks && block->TestUse())
        || (alwaysClipBlocks != NULL && alwaysClipBlocks->find(block) != alwaysClipBlocks->end()))
    {
        if (block->mayPick(state, stopOnLiquid))
        {
            HitResult* hit = state->clip(this, current, from, to);

            if (hit != NULL)
            {
                return hit;
            }
        }
        else if (extraBlock->mayPick(extraState, stopOnLiquid))
        {
            HitResult* hit = extraState->clip(this, current, from, to);

            if (hit != NULL)
            {
                return hit;
            }
        }
    }

    HitResult* lastUncollidable = NULL;
    int steps = 199;

    while (true)
    {
        if (currentX == targetX && currentY == targetY && currentZ == targetZ)
        {
            if (returnLastUncollidableBlock)
            {
                return lastUncollidable;
            }

            delete lastUncollidable;
            return NULL;
        }

        bool crossX = true;
        bool crossY = true;
        bool crossZ = true;
        double planeX = 999.0;
        double planeY = 999.0;
        double planeZ = 999.0;

        if (targetX > currentX)
        {
            planeX = (double)currentX + 1.0;
        }
        else if (targetX < currentX)
        {
            planeX = (double)currentX;
        }
        else
        {
            crossX = false;
        }

        if (targetY > currentY)
        {
            planeY = (double)currentY + 1.0;
        }
        else if (targetY < currentY)
        {
            planeY = (double)currentY;
        }
        else
        {
            crossY = false;
        }

        if (targetZ > currentZ)
        {
            planeZ = (double)currentZ + 1.0;
        }
        else if (targetZ < currentZ)
        {
            planeZ = (double)currentZ;
        }
        else
        {
            crossZ = false;
        }

        double deltaX = to->x - from->x;
        double deltaY = to->y - from->y;
        double deltaZ = to->z - from->z;
        double partX = 999.0;
        double partY = 999.0;
        double partZ = 999.0;

        if (crossX)
        {
            partX = (planeX - from->x) / deltaX;
        }

        if (crossY)
        {
            partY = (planeY - from->y) / deltaY;
        }

        if (crossZ)
        {
            partZ = (planeZ - from->z) / deltaZ;
        }

        if (partX == -0.0)
        {
            partX = -1.0E-4;
        }

        if (partY == -0.0)
        {
            partY = -1.0E-4;
        }

        if (partZ == -0.0)
        {
            partZ = -1.0E-4;
        }

        const Direction* side;

        if (partX < partY && partX < partZ)
        {
            side = (targetX > currentX) ? Direction::WEST : Direction::EAST;
            from = Vec3::newTemp(planeX, from->y + deltaY * partX, from->z + deltaZ * partX);
        }
        else if (partY < partZ)
        {
            side = (targetY > currentY) ? Direction::DOWN : Direction::UP;
            from = Vec3::newTemp(from->x + deltaX * partY, planeY, from->z + deltaZ * partY);
        }
        else
        {
            side = (targetZ > currentZ) ? Direction::NORTH : Direction::SOUTH;
            from = Vec3::newTemp(from->x + deltaX * partZ, from->y + deltaY * partZ, planeZ);
        }

        currentX = Mth::fastFloor(from->x) - ((side == Direction::EAST) ? 1 : 0);
        currentY = Mth::fastFloor(from->y) - ((side == Direction::UP) ? 1 : 0);
        currentZ = Mth::fastFloor(from->z) - ((side == Direction::SOUTH) ? 1 : 0);
        current.set(currentX, currentY, currentZ);

        state = getBlockState(current);
        block = state->getBlock();
        extraState = getExtraBlockState(current);
        extraBlock = extraState->getBlock();

        if (!ignoreBlocksWithoutShape || state->getMaterial() == Material::PORTAL
            || state->getClipAABB(this, current) != NULL
            || (testUseBlocks && block->TestUse())
            || (alwaysClipBlocks != NULL && alwaysClipBlocks->find(block) != alwaysClipBlocks->end()))
        {
            if (block->mayPick(state, stopOnLiquid))
            {
                HitResult* hit = state->clip(this, current, from, to);

                if (hit != NULL)
                {
                    delete lastUncollidable;
                    return hit;
                }
            }
            else if (extraBlock->mayPick(extraState, stopOnLiquid))
            {
                HitResult* hit = extraState->clip(this, current, from, to);

                if (hit != NULL)
                {
                    return hit;
                }
            }
            else
            {
                delete lastUncollidable;
                lastUncollidable = new HitResult(HitResult::MISS, from, side, current);
            }
        }

        if (steps < 0)
        {
            break;
        }

        --steps;
    }

    if (returnLastUncollidableBlock)
    {
        return lastUncollidable;
    }

    delete lastUncollidable;
    return NULL;
}

HitResult* Level::clip(Vec3* from, Vec3* to)
{
    return clip(from, to, false, false, false, false, NULL, false);
}

HitResult* Level::clip(Vec3* from, Vec3* to, bool stopOnLiquid)
{
    return clip(from, to, stopOnLiquid, false, false, false, NULL, false);
}

void Level::playSoundToAllPlayers(const BlockPos& pos, const SoundEvent* sound,
                                  SoundSource::ESoundSource source, float volume, float pitch,
                                  float range)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->playSoundToAllPlayers(sound, source, (double)pos.x + 0.5, (double)pos.y + 0.5,
                                     (double)pos.z + 0.5, volume, pitch, range);
    }
}

void Level::playSound(boost::shared_ptr<Player> player, const BlockPos& pos, const SoundEvent* sound,
                      SoundSource::ESoundSource source, float volume, float pitch, float range)
{
    playSound(player, (double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5, sound, source,
              volume, pitch, 16.0f);
}

void Level::playSound(boost::shared_ptr<Player> player, double x, double y, double z,
                      const SoundEvent* sound, SoundSource::ESoundSource source, float volume,
                      float pitch, float range)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->playSound(player, sound, source, x, y, z, volume, pitch, range);
    }
}

void Level::playLocalSound(double x, double y, double z, const SoundEvent* sound,
                           SoundSource::ESoundSource source, float volume, float pitch, bool distanceDelay,
                           float range)
{
}

void Level::playStreamingMusic(const BlockPos& pos, const SoundEvent* sound, bool playing)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->playStreamingMusic(sound, pos, playing);
    }
}

void Level::addParticle(ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z,
                        double xa, double ya, double za, arrayWithLength<int> params)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->addParticle(this, type, overrideLimiter, x, y, z, xa, ya, za, params);
    }
}

void Level::addParticle(const ParticleType* type, double x, double y, double z, double xa, double ya,
                        double za, arrayWithLength<int> params)
{
    addParticle(type->getId(), type->getOverrideLimiter(), x, y, z, xa, ya, za, params);
}

void Level::addAlwaysVisibleParticle(ePARTICLE_TYPE type, double x, double y, double z, double xa,
                                     double ya, double za, arrayWithLength<int> params)
{
    for (unsigned int i = 0; i < mListeners.size(); ++i)
    {
        mListeners[i]->addParticle(this, type, false, true, x, y, z, xa, ya, za, params);
    }
}

void Level::addParticle(const ParticleType* type, bool overrideLimiter, double x, double y, double z,
                        double xa, double ya, double za, arrayWithLength<int> params)
{
    addParticle(type->getId(), type->getOverrideLimiter() || overrideLimiter, x, y, z, xa, ya, za,
                params);
}

bool Level::addGlobalEntity(boost::shared_ptr<Entity> entity)
{
    mGlobalEntities.push_back(entity);
    return true;
}

bool Level::addEntity(boost::shared_ptr<Entity> entity)
{
    entity->fjCheckDerivedConstruction();

    int chunkX = Mth::fastFloor(entity->mX * 0.0625);
    int chunkZ = Mth::fastFloor(entity->mZ * 0.0625);
    bool forcedLoading = entity->mForcedLoading;

    if ((entity->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER && !forcedLoading
        && !hasChunk(chunkX, chunkZ, false))
    {
        return false;
    }

    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin();

        while (it != mPlayers.end() && *it != player)
        {
            ++it;
        }

        if (it == mPlayers.end())
        {
            mPlayers.push_back(player);
        }

        updateSleepingPlayerList();
    }

    MemSect(0x2A);
    getChunk(chunkX, chunkZ)->addEntity(entity);
    MemSect(0);

    EnterCriticalSection(&mEntityMutex);
    MemSect(0x2B);
    mEntities.push_back(entity);
    MemSect(0);
    LeaveCriticalSection(&mEntityMutex);

    MemSect(0x2C);
    entityAdded(entity);
    MemSect(0);
    return true;
}

void Level::entityAdded(boost::shared_ptr<Entity> entity)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->entityAdded(entity);
    }
}

void Level::entityRemoved(boost::shared_ptr<Entity> entity)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->entityRemoved(entity);
    }
}

void Level::playerRemoved(boost::shared_ptr<Entity> player)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->playerRemoved(player);
    }
}

void Level::removeEntity(boost::shared_ptr<Entity> entity)
{
    if (entity->isVehicle())
    {
        entity->ejectPassengers();
    }

    if (entity->isPassenger())
    {
        entity->stopRiding();
    }

    entity->remove();

    if ((entity->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
    {
        return;
    }

    std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin();

    while (it != mPlayers.end() && *it != boost::static_pointer_cast<Player>(entity))
    {
        ++it;
    }

    if (it != mPlayers.end())
    {
        mPlayers.erase(it);
    }

    updateSleepingPlayerList();
    playerRemoved(entity);
}

void Level::removeEntityImmediately(boost::shared_ptr<Entity> entity)
{
    entity->setDropContainerContent(false);
    entity->remove();

    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        std::vector<boost::shared_ptr<Player> >::iterator playerIt = mPlayers.begin();

        while (playerIt != mPlayers.end() && *playerIt != boost::static_pointer_cast<Player>(entity))
        {
            ++playerIt;
        }

        if (playerIt != mPlayers.end())
        {
            mPlayers.erase(playerIt);
        }

        updateSleepingPlayerList();
        playerRemoved(entity);
    }

    if (entity->mAddedToChunk && hasChunk(entity->mChunkX, entity->mChunkZ, true))
    {
        getChunk(entity->mChunkX, entity->mChunkZ)->removeEntity(entity);
    }

    EnterCriticalSection(&mEntityMutex);

    std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin();

    while (it != mEntities.end() && *it != entity)
    {
        ++it;
    }

    if (it != mEntities.end())
    {
        mEntities.erase(it);
    }

    LeaveCriticalSection(&mEntityMutex);
    entityRemoved(entity);
}

void Level::addListener(LevelListener* listener)
{
    mListeners.push_back(listener);
}

void Level::removeListener(LevelListener* listener)
{
    std::vector<LevelListener*>::iterator it = mListeners.begin();

    while (it != mListeners.end() && *it != listener)
    {
        ++it;
    }

    if (it == mListeners.end())
    {
        return;
    }

    mListeners.erase(it);
}

bool Level::isInsideWorldBorder(boost::shared_ptr<Entity> entity)
{
    const AABB* constraints = mDimension->getPlayerConstraints(BlockPos(entity));
    double minX = constraints->mMinX;
    double maxX = constraints->mMaxX;
    double minZ = constraints->mMinZ;
    double maxZ = constraints->mMaxZ;

    if (entity->mOutsideWorldBorder)
    {
        minX += 1.0;
        maxX -= 1.0;
        minZ += 1.0;
        maxZ -= 1.0;
    }
    else
    {
        minX -= 1.0;
        maxX += 1.0;
        minZ -= 1.0;
        maxZ += 1.0;
    }

    return entity->mX > minX && entity->mX < maxX && entity->mZ > minZ && entity->mZ < maxZ;
}

bool Level::checkAndHandleWater(AABB* box, Material* material, boost::shared_ptr<Entity> entity)
{
    int minX = (int)std::floor(box->mMinX);
    int maxX = (int)std::ceil(box->mMaxX);
    int minY = (int)std::floor(box->mMinY);
    int maxY = (int)std::ceil(box->mMaxY);
    int minZ = (int)std::floor(box->mMinZ);
    int maxZ = (int)std::ceil(box->mMaxZ);

    if (!hasChunksAt(minX, minY, minZ, maxX, maxY, maxZ, true))
    {
        return false;
    }

    bool found = false;
    Vec3* flow = Vec3::newTemp(0.0, 0.0, 0.0);

    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                BlockPos pos(x, y, z);
                const BlockState* state = getBlockState(entity->mTickCache, pos);
                Block* block = state->getBlock();

                if (state->getMaterial() == material)
                {
                    int data = block->convertBlockStateToLegacyData(state);
                    double surface = (double)(y + 1) - LiquidBlock::getHeightFromData(data);

                    if (surface <= (double)maxY)
                    {
                        found = true;
                        flow = block->handleEntityInside(this, pos, entity, flow);
                    }
                }
            }
        }
    }

    if (flow->length() > 0.0 && entity->isPushedByWater())
    {
        flow = flow->normalize();
        entity->push(flow->x * 0.014, flow->y * 0.014, flow->z * 0.014);
    }

    return found;
}

float Level::getTimeOfDay(float partialTick)
{
    long long dayTime = mLevelData->getDayTime();
    float angle = (float)((dayTime % 24000) + partialTick) / 24000.0f - 0.25f;
    if (angle < 0.0f)
    {
        angle += 1.0f;
    }
    if (angle > 1.0f)
    {
        angle -= 1.0f;
    }
    float cosine = (float)std::cos((double)angle * 3.14159265358979323846);
    float adjusted = angle + (0.5f - cosine) / 3.0f;
    return adjusted;
}

float Level::getRainLevel(float partialTick)
{
    return mOldRainLevel + (mRainLevel - mOldRainLevel) * partialTick;
}

float Level::getThunderLevel(float partialTick)
{
    return mOldThunderLevel + (mThunderLevel - mOldThunderLevel) * partialTick;
}

float Level::getOldSkyDarken(float partialTick)
{
    return (float)mSkyDarken;
}

float Level::getSkyDarken(float partialTick)
{
    return (float)mSkyDarken;
}

int Level::getMoonPhase()
{
    return (int)((mLevelData->getDayTime() / 24000) % 8);
}

float Level::getMoonBrightness()
{
    static const float brightness[8] = {1.0f, 0.75f, 0.5f, 0.25f, 0.0f, 0.25f, 0.5f, 0.75f};
    return brightness[getMoonPhase()];
}

float Level::getSunAngle(float partialTick)
{
    return getTimeOfDay(partialTick) * 6.28318530717958647692f;
}

int Level::getCloudColor(float partialTick)
{
    return mCloudColour;
}

bool Level::biomeHasRain(const BlockPos& pos)
{
    return getBiome(pos)->hasRain();
}

bool Level::biomeHasSnow(const BlockPos& pos)
{
    return getBiome(pos)->getTemperature(pos) < 0.15f;
}

float Level::getStarBrightness(float partialTick)
{
    float sky = 1.0f - getSkyDarken(partialTick) / 15.0f;
    return sky * sky * 0.5f;
}

const Difficulty* Level::getDifficulty()
{
    return mLevelData->getDifficulty();
}

DifficultyInstance Level::getCurrentDifficultyAt(const BlockPos& pos)
{
    long long inhabitedTime = 0;
    if (hasChunkAt(pos))
    {
        inhabitedTime = getChunkAt(pos)->getInhabitedTime();
    }
    return DifficultyInstance(getDifficulty(), mLevelData->getDayTime(), inhabitedTime, getMoonBrightness());
}

int Level::getSkyDarken()
{
    return mSkyDarken;
}

void Level::setSkyDarken(int value)
{
    mSkyDarken = value;
}

int Level::getSkyFlashTime()
{
    return mSkyFlashTime;
}

void Level::setSkyFlashTime(int value)
{
    mSkyFlashTime = value;
}

bool Level::isFindingSpawn()
{
    return mFindingSpawn;
}

void Level::setFindingSpawn(bool value)
{
    mFindingSpawn = value;
}

GameRules* Level::getGameRules()
{
    return mLevelData->getGameRules();
}

int Level::getMaxBuildHeight()
{
    return 256;
}

int Level::getHeight()
{
    return getMaxBuildHeight();
}

WorldBorder* Level::getWorldBorder()
{
    return mWorldBorder;
}

int Level::getCloudHeight()
{
    return mLevelData->getCloudHeight();
}

int Level::getFogDistance()
{
    return mFogDistance;
}

void Level::setFogDistance(int value)
{
    mFogDistance = value;
}

bool Level::useNewSeaLevel()
{
    return mLevelData->useNewSeaLevel();
}

bool Level::isFlatLand()
{
    return mLevelData->getGeneratorType() == LevelType::FLAT;
}

bool Level::AllPlayersAreSleeping()
{
    if (mPlayers.empty())
    {
        return false;
    }
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        if (!(*it)->isSleeping())
        {
            return false;
        }
    }
    return true;
}

bool Level::isSpawnChunk(int chunkX, int chunkZ)
{
    int spawnX = mLevelData->getXSpawn() >> 4;
    int spawnZ = mLevelData->getZSpawn() >> 4;
    return chunkX >= spawnX - 1 && chunkX <= spawnX + 1 && chunkZ >= spawnZ - 1 && chunkZ <= spawnZ + 1;
}

bool Level::getHasBeenInCreative()
{
    return mLevelData->getHasBeenInCreative();
}

bool Level::isGenerateMapFeatures()
{
    return mLevelData->isGenerateMapFeatures();
}

BlockPos Level::getTopRainBlockPos(const BlockPos& pos)
{
    return getHeightmapPos(pos);
}

bool Level::isSnowTemperature(const BlockPos& pos)
{
    return getBiome(pos)->getTemperature(pos) < 0.15f;
}

bool Level::isRainingAt(const BlockPos& pos)
{
    if (!isRaining() || !canSeeSky(pos))
    {
        return false;
    }
    BlockPos top = getTopRainBlockPos(pos);
    return top.y <= pos.y && !biomeHasSnow(pos) && !isSnowTemperature(pos) && biomeHasRain(pos);
}

bool Level::isSnowingAt(const BlockPos& pos)
{
    if (!isRaining() || !canSeeSky(pos))
    {
        return false;
    }
    return getTopRainBlockPos(pos).y <= pos.y && isSnowTemperature(pos);
}

void Level::setThunderLevel(float value)
{
    mOldThunderLevel = value;
    mThunderLevel = value;
}

void Level::setRainLevel(float value)
{
    mOldRainLevel = value;
    mRainLevel = value;
}

bool Level::isThundering()
{
    return getThunderLevel(1.0f) > 0.9f;
}

bool Level::isRaining()
{
    return getRainLevel(1.0f) > 0.2f;
}

void Level::setSavedDataStorage(SavedDataStorage* storage)
{
    mSavedDataStorage = storage;
}

SavedDataStorage* Level::getSavedDataStorage()
{
    return mSavedDataStorage;
}

void Level::setSavedData(const std::wstring& id, boost::shared_ptr<SavedData> data)
{
    if (mSavedDataStorage != NULL)
    {
        mSavedDataStorage->set(id, data);
    }
}

boost::shared_ptr<SavedData> Level::getSavedData(const std::type_info& type, const std::wstring& id)
{
    if (mSavedDataStorage == NULL)
    {
        return boost::shared_ptr<SavedData>();
    }
    return mSavedDataStorage->get(type, id);
}

int Level::getFreeAuxValueFor(const std::wstring& id)
{
    return mSavedDataStorage == NULL ? -1 : mSavedDataStorage->getFreeAuxValueFor(id);
}

int Level::getAuxValueForMap(PlayerUID uid, int dimension, int x, int z, int scale)
{
    return mSavedDataStorage == NULL ? -1 : mSavedDataStorage->getAuxValueForMap(uid, dimension, x, z, scale);
}

void Level::globalLevelEvent(int type, const BlockPos& pos, int data)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->globalLevelEvent(type, pos, data);
    }
}

Random Level::getRandomFor(int x, int y, int z)
{
    return fjGetRandomFor(getSeed(), x, y, z);
}

Random Level::fjGetRandomFor(long long seed, int x, int y, int z)
{
    long long mixed = seed + (long long)x * 341873128712LL + (long long)y * 132897987541LL
                      + (long long)z * 42317861LL;
    return Random(mixed);
}

bool Level::isAllEmpty()
{
    return mEntities.empty() && mGlobalEntities.empty();
}

int Level::getHorizonHeight()
{
    return mLevelData->getYSpawn();
}

void Level::destroyBlockProgress(int breakerId, const BlockPos& pos, int progress)
{
    for (std::vector<LevelListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->destroyBlockProgress(breakerId, pos, progress);
    }
}

void Level::setSeaLevel(int value)
{
    mSeaLevel = value;
}

LevelType* Level::getGeneratorType()
{
    return mLevelData->getGeneratorType();
}

void Level::setSpawnSettings(bool spawnPeaceful, bool spawnHostile)
{
    mSpawnPeacefulMobs = spawnPeaceful;
    mSpawnHostileMobs = spawnHostile;
}

int Level::getDirectSignal(const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = getBlockState(pos);
    return state->getBlock()->getDirectSignal(state, this, pos, direction);
}

int Level::getDirectSignalTo(const BlockPos& pos)
{
    int signal = 0;
    static const Direction* directions[6] = {Direction::DOWN, Direction::UP, Direction::NORTH,
                                             Direction::SOUTH, Direction::WEST, Direction::EAST};
    for (int i = 0; i < 6; ++i)
    {
        int value = getDirectSignal(pos.relative(directions[i]), directions[i]);
        if (value > signal)
        {
            signal = value;
        }
    }
    return signal;
}

int Level::getSignal(const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = getBlockState(pos);
    return state->getBlock()->getSignal(state, this, pos, direction);
}

bool Level::hasSignal(const BlockPos& pos, const Direction* direction)
{
    return getSignal(pos, direction) > 0;
}

bool Level::hasNeighborSignal(const BlockPos& pos)
{
    return getBestNeighborSignal(pos) > 0;
}

int Level::getBestNeighborSignal(const BlockPos& pos)
{
    int signal = 0;
    static const Direction* directions[6] = {Direction::DOWN, Direction::UP, Direction::NORTH,
                                             Direction::SOUTH, Direction::WEST, Direction::EAST};
    for (int i = 0; i < 6; ++i)
    {
        int value = getSignal(pos.relative(directions[i]), directions[i]);
        if (value > signal)
        {
            signal = value;
        }
    }
    return signal;
}

boost::shared_ptr<Player> Level::getNearestPlayer(boost::shared_ptr<Entity> entity, double distance,
                                                   double maxYDistance)
{
    return getNearestPlayer(entity, distance, false, maxYDistance);
}

boost::shared_ptr<Player> Level::getNearestPlayer(boost::shared_ptr<Entity> entity, double distance,
                                                   bool includeCreative, double maxYDistance)
{
    if (entity == NULL)
    {
        return boost::shared_ptr<Player>();
    }

    boost::shared_ptr<Player> nearest;
    double nearestDistance = -1.0;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<Player> player = *it;
        if (player == NULL || (!includeCreative && player->isCreative()))
        {
            continue;
        }

        double currentDistance = entity->distanceToSqr(player);
        if ((distance < 0.0 || currentDistance < distance * distance)
            && (maxYDistance < 0.0 || std::fabs(entity->mY - player->mY) <= maxYDistance)
            && (nearestDistance < 0.0 || currentDistance < nearestDistance))
        {
            nearest = player;
            nearestDistance = currentDistance;
        }
    }
    return nearest;
}

boost::shared_ptr<Player> Level::getNearestPlayer(double x, double y, double z, double distance,
                                                   bool includeCreative, double maxYDistance)
{
    boost::shared_ptr<Player> nearest;
    double nearestDistance = -1.0;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<Player> player = *it;
        if (player == NULL || (!includeCreative && player->isCreative()))
        {
            continue;
        }

        double currentDistance = player->distanceToSqr(x, y, z);
        if ((distance < 0.0 || currentDistance < distance * distance)
            && (maxYDistance < 0.0 || std::fabs(player->mY - y) <= maxYDistance)
            && (nearestDistance < 0.0 || currentDistance < nearestDistance))
        {
            nearest = player;
            nearestDistance = currentDistance;
        }
    }
    return nearest;
}

boost::shared_ptr<Player> Level::getNearestPlayer(double x, double y, double z)
{
    boost::shared_ptr<Player> nearest;
    double nearestDistance = -1.0;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<Player> player = *it;
        if (player == NULL)
        {
            continue;
        }
        double currentDistance = player->distanceToSqr(x, y, z);
        if (nearestDistance < 0.0 || currentDistance < nearestDistance)
        {
            nearest = player;
            nearestDistance = currentDistance;
        }
    }
    return nearest;
}

boost::shared_ptr<Player> Level::getNearestPlayerNotCreative(boost::shared_ptr<Entity> entity, double distance,
                                                              double maxYDistance)
{
    return getNearestPlayer(entity, distance, false, maxYDistance);
}

boost::shared_ptr<Player> Level::getNearestPlayerPredicate(
    boost::shared_ptr<Entity> entity, double distance,
    const Predicate<boost::shared_ptr<Entity> >* predicate)
{
    boost::shared_ptr<Player> nearest;
    double nearestDistance = -1.0;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<Player> player = *it;
        if (player == NULL || player->mDimension != entity->mDimension
            || (predicate != NULL && !predicate->apply(boost::static_pointer_cast<Entity>(player))))
        {
            continue;
        }
        double currentDistance = player->distanceToSqr(entity->mX, entity->mY, entity->mZ);
        if ((distance < 0.0 || currentDistance < distance * distance)
            && (nearestDistance < 0.0 || currentDistance < nearestDistance)
            && player->isAlive())
        {
            nearest = player;
            nearestDistance = currentDistance;
        }
    }
    return nearest;
}

boost::shared_ptr<Player> Level::getNearestPlayerPredicate(double x, double y, double z, double distance,
                                                            const Predicate<boost::shared_ptr<Entity> >* predicate)
{
    boost::shared_ptr<Player> nearest;
    double nearestDistance = -1.0;
    for (std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it)
    {
        boost::shared_ptr<Player> player = *it;
        if (player == NULL || (predicate != NULL
            && !predicate->apply(boost::static_pointer_cast<Entity>(player))))
        {
            continue;
        }

        double currentDistance = player->distanceToSqr(x, y, z);
        if ((distance < 0.0 || currentDistance < distance * distance)
            && (nearestDistance < 0.0 || currentDistance < nearestDistance))
        {
            nearest = player;
            nearestDistance = currentDistance;
        }
    }
    return nearest;
}

void Level::setGameTime(long long value)
{
    mLevelData->setGameTime(value);
}

long long Level::getGameTime()
{
    return mLevelData->getGameTime();
}

long long Level::getDayTime()
{
    return mLevelData->getDayTime();
}

void Level::setDayTime(long long value)
{
    mLevelData->setDayTime(value);
}

void Level::forceDayTime(long long value)
{
    mLevelData->setDayTime(value);
    mForceTimeUpdate = true;
}

bool Level::ShouldForceTimeUpdate()
{
    bool force = mForceTimeUpdate;
    mForceTimeUpdate = false;
    return force;
}

BlockPos Level::getSharedSpawnPos()
{
    return BlockPos(mLevelData->getXSpawn(), mLevelData->getYSpawn(), mLevelData->getZSpawn());
}

ChunkSource* Level::getChunkSource()
{
    return mChunkSource;
}

boost::shared_ptr<LevelStorage> Level::getLevelStorage()
{
    return mLevelStorage;
}

unsigned char Level::getWeatherState()
{
    if (isThundering())
    {
        return 2;
    }
    return isRaining() ? 1 : 0;
}

bool Level::hasTickNextTick(const BlockPos&, Block*)
{
    return true;
}

void Level::addToTickNextTick(const BlockPos&, Block*, int)
{
    return;
}

void Level::addToTickNextTick(const BlockPos&, Block*, int, int)
{
    return;
}

void Level::forceAddBlockTick(const BlockPos&, Block*, int, int)
{
    return;
}

bool Level::newPrimedTntAllowed()
{
    return true;
}

BlockPos Level::getTopSolidBlockPos(const BlockPos& pos)
{
    LevelChunk* chunk = getChunkAt(pos);
    BlockPos current(pos.x, chunk->getHighestSectionPosition() + 16, pos.z);
    while (current.y >= 1)
    {
        const BlockState* state = chunk->getBlockState(current);
        if (state->getBlock()->isSolidBlockingCube(state))
        {
            return current;
        }
        current = current.below();
    }
    return current;
}

bool Level::shouldFreeze(const BlockPos& pos)
{
    return shouldFreeze(pos, true);
}

void Level::tickPendingTicks(bool runAll)
{
}

void Level::fetchTicksInChunk(LevelChunk* chunk, bool runAll)
{
}

void Level::fetchTicksInArea(BoundingBox* box, bool runAll)
{
}

void Level::updateSleepingPlayerList()
{
}

void* Level::getLootTables()
{
    return mLootTables;
}

bool Level::newFallingBlockAllowed()
{
    return true;
}

bool Level::explodeNextEnderCrystal()
{
    return true;
}

void Level::disconnect(bool save)
{
}

void Level::checkSession()
{
    if (mLevelStorage != NULL)
    {
        mLevelStorage->checkSession();
    }
}

void Level::broadcastEntityEvent(boost::shared_ptr<Entity> entity, unsigned char event, int data)
{
}

void Level::sendPacketToServer(boost::shared_ptr<Packet>)
{
    return;
}

boost::shared_ptr<Village> Level::getVillages()
{
    return mVillages;
}

BlockPos* Level::findNearestMapFeature(const std::wstring& feature, const BlockPos& pos, bool findUnexplored)
{
    if (mChunkSource == NULL)
    {
        return NULL;
    }
    return mChunkSource->findNearestMapFeature(this, feature, pos, findUnexplored);
}

bool Level::isHumidAt(const BlockPos& pos)
{
    return getBiome(pos)->isHumid();
}

int Level::getAuxValueForMap(PlayerUID uid, int dimension, const BlockPos& pos, int scale)
{
    return getAuxValueForMap(uid, dimension, pos.x, pos.z, scale);
}

void Level::blockEntityAddedExtra(boost::shared_ptr<BlockEntity>)
{
    return;
}

void Level::blockEntityRemovedExtra(boost::shared_ptr<BlockEntity>)
{
    return;
}

std::vector<boost::shared_ptr<Player> >* Level::getPlayers(unsigned long long,
                                                             const Predicate<boost::shared_ptr<Player> >* predicate)
{
    std::vector<boost::shared_ptr<Player> >* result = new std::vector<boost::shared_ptr<Player> >();
    std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        boost::shared_ptr<Player> player = *it;
        if (player != NULL && (predicate == NULL || predicate->apply(player)))
        {
            result->push_back(player);
        }
        ++it;
    }
    return result;
}

std::vector<boost::shared_ptr<Entity> >* Level::getAllEntities()
{
    return &mEntities;
}

boost::shared_ptr<Entity> Level::getEntity(int id)
{
    std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin();
    while (it != mEntities.end())
    {
        if (*it != NULL && (*it)->getId() == id)
        {
            return *it;
        }
        ++it;
    }
    return boost::shared_ptr<Entity>();
}

bool Level::isFullAABBBlock(const BlockPos& pos)
{
    const BlockState* state = getBlockState(pos);
    const AABB* shape = state == NULL ? NULL : state->getBlock()->getShape(state, this, pos);
    return shape != NULL && shape->getSize() >= 1.0;
}

bool Level::isSolidBlockingBlock(const BlockPos& pos)
{
    const BlockState* state = getBlockState(pos);
    return state != NULL && state->getBlock()->isSolidBlockingCube(state);
}

bool Level::isConsideredSolidBlock(const BlockPos& pos)
{
    return isSolidBlockingBlock(pos);
}

bool Level::isConsideredSolidBlock(int x, int y, int z)
{
    return isConsideredSolidBlock(BlockPos(x, y, z));
}

void Level::updateSkyBrightness()
{
    mSkyDarken = static_cast<int>(getOldSkyDarken(0.0f));
}

int Level::getAboveTopSolidBlock(const BlockPos& pos, bool, bool)
{
    return getTopSolidBlockPos(pos).y;
}

bool Level::isUnderWater(const BlockPos& pos, const BlockState* state)
{
    Block* block = state->getBlock();
    bool notCubeShaped = !block->isCubeShaped(state);
    bool fullBlock = block->mFullBlock;
    bool notSlab = !block->isSlabBlock();
    bool topSlab = false;
    if (block->isSlabBlock())
    {
        topSlab = !HalfSlabBlock::isBottomSlab(block->convertBlockStateToLegacyData(state));
    }

    Material* above = Block::byId(LiquidBlock::getLiquidElement(this, pos.above()) >> 4)->mMaterial;
    Material* below = Block::byId(LiquidBlock::getLiquidElement(this, pos.below()) >> 4)->mMaterial;
    Material* north = Block::byId(LiquidBlock::getLiquidElement(this, pos.north()) >> 4)->mMaterial;
    Material* south = Block::byId(LiquidBlock::getLiquidElement(this, pos.south()) >> 4)->mMaterial;
    Material* east = Block::byId(LiquidBlock::getLiquidElement(this, pos.east()) >> 4)->mMaterial;
    Material* west = Block::byId(LiquidBlock::getLiquidElement(this, pos.west()) >> 4)->mMaterial;
    Material* center = Block::byId(LiquidBlock::getLiquidElement(this, pos) >> 4)->mMaterial;

    if (notCubeShaped && notSlab && above == Material::WATER
        && below != Material::AIR && north != Material::AIR && south != Material::AIR
        && east != Material::AIR && west != Material::AIR)
    {
        return true;
    }
    if (!fullBlock)
    {
        if (notSlab && center == Material::WATER)
        {
            return true;
        }
        if (notCubeShaped)
        {
            return above == Material::WATER;
        }
        if (notSlab)
        {
            return false;
        }
    }
    else
    {
        if (notCubeShaped)
        {
            return false;
        }
        if (notSlab)
        {
            return north != Material::AIR && south != Material::AIR && east != Material::AIR
                && west != Material::AIR && below != Material::AIR && above != Material::AIR;
        }
    }
    return center == Material::WATER && (!topSlab || above != Material::AIR)
        && north != Material::AIR && south != Material::AIR && east != Material::AIR
        && west != Material::AIR && below != Material::AIR;
}

bool Level::isUnderWater(const BlockPos& pos, int blockId, int data)
{
    Block* block = Block::byId(blockId);
    return isUnderWater(pos, block->getBlockState(data));
}

void Level::tick(boost::shared_ptr<Entity> entity)
{
    tick(entity, true);
}

bool Level::addBlockEntity(boost::shared_ptr<BlockEntity> entity)
{
    if (entity == NULL || entity->isRemoved())
    {
        return false;
    }
    AddToBlockEntityList(entity);
    return true;
}

void Level::AddToBlockEntityList(boost::shared_ptr<BlockEntity> entity)
{
    if (mProcessingBlockEntities)
    {
        mPendingBlockEntities.push_back(entity);
    }
    else
    {
        mBlockEntities.push_back(entity);
        blockEntityAddedExtra(entity);
    }
}

void Level::tickServerPlayers()
{
}

void Level::addAllPendingBlockEntities(std::vector<boost::shared_ptr<BlockEntity> >& entities)
{
    std::vector<boost::shared_ptr<BlockEntity> >::iterator it = entities.begin();
    while (it != entities.end())
    {
        addBlockEntity(*it);
        ++it;
    }
    entities.clear();
}

void Level::tick(boost::shared_ptr<Entity> entity, bool updateChunk)
{
    if (entity == NULL || entity->mRemoved)
    {
        return;
    }
    if (entity->shouldTick())
    {
        entity->tick();
    }
    if (updateChunk && !entity->mRemoved)
    {
        ensureAdded(entity);
    }
}

void Level::tickEntities()
{
    std::vector<boost::shared_ptr<Entity> >::iterator entity = mGlobalEntities.begin();
    while (entity != mGlobalEntities.end())
    {
        tick(*entity, false);
        ++entity;
    }
    entity = mEntities.begin();
    while (entity != mEntities.end())
    {
        tick(*entity, true);
        ++entity;
    }
    mProcessingBlockEntities = true;
    std::vector<boost::shared_ptr<BlockEntity> >::iterator blockEntity = mBlockEntities.begin();
    while (blockEntity != mBlockEntities.end())
    {
        if (*blockEntity != NULL && !(*blockEntity)->isRemoved())
        {
            (*blockEntity)->tick();
        }
        ++blockEntity;
    }
    mProcessingBlockEntities = false;
    addAllPendingBlockEntities(mPendingBlockEntities);
}

void Level::addEntities(std::vector<boost::shared_ptr<Entity> >* entities)
{
    if (entities == NULL)
    {
        return;
    }
    std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
    while (it != entities->end())
    {
        addEntity(*it);
        ++it;
    }
}

void Level::removeEntities(std::vector<boost::shared_ptr<Entity> >* entities)
{
    if (entities == NULL)
    {
        return;
    }
    std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
    while (it != entities->end())
    {
        removeEntity(*it);
        ++it;
    }
}

void Level::ensureAdded(boost::shared_ptr<Entity> entity)
{
    if (entity == NULL)
    {
        return;
    }
    if (std::find(mEntities.begin(), mEntities.end(), entity) == mEntities.end())
    {
        mEntities.push_back(entity);
        entityAdded(entity);
    }
}

bool Level::mayInteract(boost::shared_ptr<Player>, const BlockPos&, Block*)
{
    return true;
}

void Level::blockEvent(const BlockPos& pos, Block* block, int event, int data)
{
    if (getBlock(pos) == block)
    {
        block->triggerEvent(getBlockState(pos), this, pos, event, data);
    }
}

void Level::createFireworks(double, double, double, double, double, double, CompoundTag*)
{
    return;
}

void Level::gatherChunkSourceStats()
{
    mChunkSource->gatherStats();
}

int Level::getSaveVersion()
{
    return mLevelStorage->getSaveFile()->getSaveVersion();
}

int Level::getOriginalSaveVersion()
{
    return mLevelStorage->getSaveFile()->getOriginalSaveVersion();
}

void Level::tickClientSideBlocks(int, int, LevelChunk*)
{
    return;
}

void Level::tickBlocks()
{
    tickPendingTicks(false);
}

void Level::instaTick(const BlockPos& pos, const BlockState* state, Random& random)
{
    setInstaTick(true);
    state->getBlock()->tick(this, pos, state, &random);
    setInstaTick(false);
}

boost::shared_ptr<Player> Level::getNearbyPlayers(boost::shared_ptr<Entity> entity, double distance)
{
    return getNearestPlayer(entity, distance, false, -1.0);
}

bool Level::hasNearPlayer(double x, double y, double z, double distance)
{
    return getNearestPlayer(x, y, z, distance, true, -1.0) != NULL;
}

boost::shared_ptr<Player> Level::getNearestAttackablePlayer(
    double x, double y, double z, double distance, double maxYDistance,
    const RangeModifier*, const Predicate<boost::shared_ptr<Entity> >* predicate)
{
    return getNearestPlayerPredicate(x, y, z, distance, predicate);
}

boost::shared_ptr<Player> Level::getNearestAttackablePlayer(const BlockPos& pos, double distance, double)
{
    return getNearestPlayer(pos.x + 0.5, pos.y + 0.5, pos.z + 0.5, distance, false, -1.0);
}

boost::shared_ptr<Player> Level::getPlayerByName(const std::wstring& name)
{
    std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        boost::shared_ptr<Entity> entity = boost::static_pointer_cast<Entity>(*it);
        if (entity != NULL && entity->getName() == name)
        {
            return *it;
        }
        ++it;
    }
    return boost::shared_ptr<Player>();
}

boost::shared_ptr<Player> Level::getPlayerByUUID(const std::wstring& uuid)
{
    std::vector<boost::shared_ptr<Player> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        boost::shared_ptr<Entity> entity = boost::static_pointer_cast<Entity>(*it);
        if (entity != NULL && entity->getUUID() == uuid)
        {
            return *it;
        }
        ++it;
    }
    return boost::shared_ptr<Player>();
}

void Level::blockEntityChanged(const BlockPos& pos, boost::shared_ptr<BlockEntity> entity)
{
    if (hasChunkAt(pos))
    {
        getChunkAt(pos)->setBlockEntity(pos, entity);
    }
}

boost::shared_ptr<BlockEntity> Level::getPendingBlockEntityAt(const BlockPos& pos)
{
    std::vector<boost::shared_ptr<BlockEntity> >::reverse_iterator it = mPendingBlockEntities.rbegin();
    while (it != mPendingBlockEntities.rend())
    {
        if (*it != NULL && !(*it)->isRemoved() && (*it)->getBlockPos() == pos)
        {
            return *it;
        }
        ++it;
    }
    return boost::shared_ptr<BlockEntity>();
}

boost::shared_ptr<BlockEntity> Level::getBlockEntity(const BlockPos& pos)
{
    if (isOutsideBuildHeight(pos))
    {
        return boost::shared_ptr<BlockEntity>();
    }
    boost::shared_ptr<BlockEntity> pending = getPendingBlockEntityAt(pos);
    if (pending != NULL)
    {
        return pending;
    }
    LevelChunk* chunk = getChunkAt(pos);
    return chunk == NULL ? boost::shared_ptr<BlockEntity>() : chunk->getBlockEntity(pos, LevelChunk::IMMEDIATE);
}

void Level::setBlockEntity(const BlockPos& pos, boost::shared_ptr<BlockEntity> entity)
{
    if (entity == NULL || isOutsideBuildHeight(pos))
    {
        return;
    }
    entity->setLevel(this);
    entity->setPosition(pos);
    entity->clearRemoved();
    if (mProcessingBlockEntities)
    {
        mPendingBlockEntities.push_back(entity);
    }
    else
    {
        getChunkAt(pos)->setBlockEntity(pos, entity);
        addBlockEntity(entity);
    }
}

void Level::removeBlockEntity(const BlockPos& pos)
{
    boost::shared_ptr<BlockEntity> entity = getBlockEntity(pos);
    if (entity != NULL)
    {
        entity->setRemoved();
        markForRemoval(entity);
    }
    LevelChunk* chunk = getChunkAt(pos);
    if (chunk != NULL)
    {
        chunk->removeBlockEntity(pos);
    }
}

void Level::markForRemoval(boost::shared_ptr<BlockEntity> entity)
{
    if (entity != NULL)
    {
        mBlockEntitiesToRemove.push_back(entity);
    }
}

bool Level::isSolidBlockingBlockInLoadedChunk(const BlockPos& pos, bool allowEmpty)
{
    if (isOutsideBuildHeight(pos))
    {
        return false;
    }
    LevelChunk* chunk = mChunkSource->getChunkIfLoaded(pos.x >> 4, pos.z >> 4);
    if (chunk == NULL || chunk->isEmpty())
    {
        return allowEmpty;
    }
    return isSolidBlockingBlock(pos);
}

void Level::prepareWeather()
{
    mRainLevel = mOldRainLevel = mLevelData->isRaining() ? 1.0f : 0.0f;
    mThunderLevel = mOldThunderLevel = mLevelData->isThundering() ? 1.0f : 0.0f;
}

void Level::tick()
{
    tickWeather(true);
    updateSkyBrightness();
    tickBlocks();
}

void Level::tickWeather(bool)
{
    mOldRainLevel = mRainLevel;
    mOldThunderLevel = mThunderLevel;
    float rainTarget = mLevelData->isRaining() ? 1.0f : 0.0f;
    float thunderTarget = mLevelData->isThundering() ? 1.0f : 0.0f;
    mRainLevel += rainTarget > mRainLevel ? 0.01f : -0.01f;
    mThunderLevel += thunderTarget > mThunderLevel ? 0.01f : -0.01f;
    mRainLevel = Mth::clamp(mRainLevel, 0.0f, 1.0f);
    mThunderLevel = Mth::clamp(mThunderLevel, 0.0f, 1.0f);
}

void Level::buildAndPrepareChunksToPoll()
{
    mChunkSource->tick();
}

std::vector<boost::shared_ptr<Entity> >* Level::getEntities(
    boost::shared_ptr<Entity> excluded, const AABB* box,
    const Predicate<boost::shared_ptr<Entity> >* predicate,
    std::vector<boost::shared_ptr<Entity> >* entities)
{
    std::vector<boost::shared_ptr<Entity> >* result = entities != NULL ? entities : &mEntities;
    result->clear();
    if (box == NULL)
    {
        return result;
    }
    std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin();
    while (it != mEntities.end())
    {
        boost::shared_ptr<Entity> entity = *it;
        if (entity != NULL && entity != excluded && !entity->mRemoved && entity->mBoundingBox != NULL
            && entity->mBoundingBox->intersectsInner(box)
            && (predicate == NULL || predicate->apply(entity)))
        {
            result->push_back(entity);
        }
        ++it;
    }
    return result;
}

std::vector<boost::shared_ptr<Entity> >* Level::getEntities(
    boost::shared_ptr<Entity> excluded, const AABB* box,
    std::vector<boost::shared_ptr<Entity> >* entities)
{
    return getEntities(excluded, box, NULL, entities);
}

std::vector<boost::shared_ptr<Entity> >* Level::getEntitiesOfClass(
    const std::type_info& type, const AABB* box,
    const Predicate<boost::shared_ptr<Entity> >* predicate, bool includeRemoved)
{
    std::vector<boost::shared_ptr<Entity> >* result = new std::vector<boost::shared_ptr<Entity> >();
    std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin();
    while (it != mEntities.end())
    {
        boost::shared_ptr<Entity> entity = *it;
        if (entity != NULL && (includeRemoved || !entity->mRemoved) && typeid(*entity) == type
            && (box == NULL || (entity->mBoundingBox != NULL && entity->mBoundingBox->intersectsInner(box)))
            && (predicate == NULL || predicate->apply(entity)))
        {
            result->push_back(entity);
        }
        ++it;
    }
    return result;
}

std::vector<boost::shared_ptr<Entity> >* Level::getEntitiesOfClass(const std::type_info& type,
                                                                    const AABB* box)
{
    return getEntitiesOfClass(type, box, NULL, false);
}

bool Level::hasEntitiesOfClass(const std::type_info& type, const AABB* box,
                               const Predicate<boost::shared_ptr<Entity> >* predicate,
                               bool includeRemoved)
{
    std::vector<boost::shared_ptr<Entity> >* entities = getEntitiesOfClass(type, box, predicate, includeRemoved);
    bool result = !entities->empty();
    delete entities;
    return result;
}

boost::shared_ptr<Entity> Level::getClosestEntityOfClass(const std::type_info& type, const AABB* box,
                                                         boost::shared_ptr<Entity> excluded)
{
    boost::shared_ptr<Entity> closest;
    double closestDistance = -1.0;
    std::vector<boost::shared_ptr<Entity> >* entities = getEntitiesOfClass(type, box);
    std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin();
    while (it != entities->end())
    {
        if (*it != excluded)
        {
            double distance = excluded == NULL ? 0.0 : (*it)->distanceToSqr(excluded);
            if (closestDistance < 0.0 || distance < closestDistance)
            {
                closest = *it;
                closestDistance = distance;
            }
        }
        ++it;
    }
    delete entities;
    return closest;
}

bool Level::isUnobstructed(const AABB* box, boost::shared_ptr<Entity> excluded)
{
    std::vector<boost::shared_ptr<Entity> > entities;
    getEntities(excluded, box, &entities);
    return entities.empty();
}

bool Level::isUnobstructed(const AABB* box)
{
    return isUnobstructed(box, boost::shared_ptr<Entity>());
}

bool Level::internalGetCollisionAABBs(boost::shared_ptr<Entity> entity, const AABB* box,
                                      bool includeEntities, std::vector<AABB*>* collisions,
                                      bool, bool includeUnloaded, bool includeExtraBlocks)
{
    if (box == NULL || collisions == NULL)
    {
        return false;
    }
    int minX = static_cast<int>(std::floor(box->mMinX));
    int maxX = static_cast<int>(std::floor(box->mMaxX + 1.0));
    int minY = static_cast<int>(std::floor(box->mMinY));
    int maxY = static_cast<int>(std::floor(box->mMaxY + 1.0));
    int minZ = static_cast<int>(std::floor(box->mMinZ));
    int maxZ = static_cast<int>(std::floor(box->mMaxZ + 1.0));
    for (int x = minX; x < maxX; ++x)
    {
        for (int z = minZ; z < maxZ; ++z)
        {
            for (int y = minY; y < maxY; ++y)
            {
                BlockPos pos(x, y, z);
                if (!includeUnloaded && !hasChunkAt(pos))
                {
                    continue;
                }
                const BlockState* state = getBlockState(pos);
                if (state != NULL)
                {
                    state->getBlock()->addCollisionAABBs(state, this, pos, box, collisions, entity, true);
                }
                if (includeExtraBlocks)
                {
                    const BlockState* extra = getExtraBlockState(pos);
                    if (extra != NULL)
                    {
                        extra->getBlock()->addCollisionAABBs(extra, this, pos, box, collisions, entity, true);
                    }
                }
            }
        }
    }
    if (includeEntities)
    {
        std::vector<boost::shared_ptr<Entity> > entities;
        getEntities(entity, box->grow(0.25), &entities);
        std::vector<boost::shared_ptr<Entity> >::iterator it = entities.begin();
        while (it != entities.end())
        {
            AABB* collision = (*it)->getCollideBox();
            if (collision != NULL && collision->intersectsInner(box))
            {
                collisions->push_back(collision);
            }
            ++it;
        }
    }
    return !collisions->empty();
}

std::vector<AABB*>* Level::getCollisionAABBs(boost::shared_ptr<Entity> entity, const AABB* box,
                                              bool includeEntities, bool includeBorder,
                                              bool includeExtraBlocks)
{
    mCollisionBoxes.clear();
    internalGetCollisionAABBs(entity, box, includeEntities, &mCollisionBoxes, includeBorder, false,
                              includeExtraBlocks);
    return &mCollisionBoxes;
}

bool Level::hasBlockCubes(const AABB* box)
{
    mCollisionBoxes.clear();
    internalGetCollisionAABBs(boost::shared_ptr<Entity>(), box, false, &mCollisionBoxes, false, false, true);
    return !mCollisionBoxes.empty();
}

bool Level::containsAnyBlocks(AABB* box)
{
    int minX = static_cast<int>(std::floor(box->mMinX));
    int maxX = static_cast<int>(std::floor(box->mMaxX + 1.0));
    int minY = static_cast<int>(std::floor(box->mMinY));
    int maxY = static_cast<int>(std::floor(box->mMaxY + 1.0));
    int minZ = static_cast<int>(std::floor(box->mMinZ));
    int maxZ = static_cast<int>(std::floor(box->mMaxZ + 1.0));
    for (int x = minX; x < maxX; ++x)
        for (int y = minY; y < maxY; ++y)
            for (int z = minZ; z < maxZ; ++z)
                if (!isEmptyBlock(BlockPos(x, y, z)))
                    return true;
    return false;
}

bool Level::containsAnyLiquid(AABB* box)
{
    int minX = static_cast<int>(std::floor(box->mMinX));
    int maxX = static_cast<int>(std::floor(box->mMaxX + 1.0));
    int minY = static_cast<int>(std::floor(box->mMinY));
    int maxY = static_cast<int>(std::floor(box->mMaxY + 1.0));
    int minZ = static_cast<int>(std::floor(box->mMinZ));
    int maxZ = static_cast<int>(std::floor(box->mMaxZ + 1.0));
    for (int x = minX; x < maxX; ++x)
        for (int y = minY; y < maxY; ++y)
            for (int z = minZ; z < maxZ; ++z)
                if (getBlockState(BlockPos(x, y, z))->getMaterial()->isLiquid())
                    return true;
    return false;
}

bool Level::containsAnysolidBlocking(AABB* box)
{
    int minX = static_cast<int>(std::floor(box->mMinX));
    int maxX = static_cast<int>(std::floor(box->mMaxX + 1.0));
    int minY = static_cast<int>(std::floor(box->mMinY));
    int maxY = static_cast<int>(std::floor(box->mMaxY + 1.0));
    int minZ = static_cast<int>(std::floor(box->mMinZ));
    int maxZ = static_cast<int>(std::floor(box->mMaxZ + 1.0));
    for (int x = minX; x < maxX; ++x)
        for (int y = minY; y < maxY; ++y)
            for (int z = minZ; z < maxZ; ++z)
                if (isSolidBlockingBlock(BlockPos(x, y, z)))
                    return true;
    return false;
}

bool Level::containsAnyLiquid_NoLoad(AABB* box)
{
    int minX = static_cast<int>(std::floor(box->mMinX));
    int maxX = static_cast<int>(std::floor(box->mMaxX + 1.0));
    int minY = static_cast<int>(std::floor(box->mMinY));
    int maxY = static_cast<int>(std::floor(box->mMaxY + 1.0));
    int minZ = static_cast<int>(std::floor(box->mMinZ));
    int maxZ = static_cast<int>(std::floor(box->mMaxZ + 1.0));
    for (int x = minX; x < maxX; ++x)
        for (int y = minY; y < maxY; ++y)
            for (int z = minZ; z < maxZ; ++z)
            {
                BlockPos pos(x, y, z);
                if (hasChunkAt(pos) && getBlockState(pos)->getMaterial()->isLiquid())
                    return true;
            }
    return false;
}

bool Level::containsFireBlock(AABB* box)
{
    int minX = static_cast<int>(std::floor(box->mMinX));
    int maxX = static_cast<int>(std::floor(box->mMaxX + 1.0));
    int minY = static_cast<int>(std::floor(box->mMinY));
    int maxY = static_cast<int>(std::floor(box->mMaxY + 1.0));
    int minZ = static_cast<int>(std::floor(box->mMinZ));
    int maxZ = static_cast<int>(std::floor(box->mMaxZ + 1.0));
    for (int x = minX; x < maxX; ++x)
        for (int y = minY; y < maxY; ++y)
            for (int z = minZ; z < maxZ; ++z)
                if (getBlock(BlockPos(x, y, z)) == Blocks::FIRE)
                    return true;
    return false;
}

bool Level::containsFireBlock(Entity*, AABB* box)
{
    return containsFireBlock(box);
}

bool Level::containsMaterial(AABB* box, Material* material)
{
    int minX = static_cast<int>(std::floor(box->mMinX));
    int maxX = static_cast<int>(std::floor(box->mMaxX + 1.0));
    int minY = static_cast<int>(std::floor(box->mMinY));
    int maxY = static_cast<int>(std::floor(box->mMaxY + 1.0));
    int minZ = static_cast<int>(std::floor(box->mMinZ));
    int maxZ = static_cast<int>(std::floor(box->mMaxZ + 1.0));
    for (int x = minX; x < maxX; ++x)
        for (int y = minY; y < maxY; ++y)
            for (int z = minZ; z < maxZ; ++z)
                if (getBlockState(BlockPos(x, y, z))->getMaterial() == material)
                    return true;
    return false;
}

bool Level::isInLava(Entity*, AABB* box)
{
    return containsMaterial(box, Material::LAVA);
}

bool Level::containsLiquid(AABB* box, Material* material)
{
    return material != NULL && material->isLiquid() && containsMaterial(box, material);
}

bool Level::canProvideSupport(const BlockPos& pos, const Direction* direction,
                              EBlockSupportType supportType)
{
    const BlockState* state = getBlockState(pos);
    return state->getBlock()->canProvideSupport(state, direction, supportType);
}

bool Level::isTouchingMaterial(const BlockPos& pos, const Material* material)
{
    return getBlockState(pos)->getMaterial() == material
        || getBlockState(pos.above())->getMaterial() == material
        || getBlockState(pos.below())->getMaterial() == material
        || getBlockState(pos.relative(Direction::NORTH))->getMaterial() == material
        || getBlockState(pos.relative(Direction::SOUTH))->getMaterial() == material
        || getBlockState(pos.relative(Direction::WEST))->getMaterial() == material
        || getBlockState(pos.relative(Direction::EAST))->getMaterial() == material;
}

int Level::countInstanceOf(unsigned long long type, bool includeRemoved,
                           unsigned int* count, unsigned int* result)
{
    unsigned int matches = 0;
    std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin();
    while (it != mEntities.end())
    {
        if (*it != NULL && (includeRemoved || !(*it)->mRemoved) && (*it)->GetType() == type)
        {
            ++matches;
        }
        ++it;
    }
    if (count != NULL)
    {
        *count = static_cast<unsigned int>(mEntities.size());
    }
    if (result != NULL)
    {
        *result = matches;
    }
    return static_cast<int>(matches);
}

int Level::countInstanceOfInRange(unsigned long long type, bool includeRemoved,
                                  int x, int y, int z, int range)
{
    int matches = 0;
    double rangeSquared = static_cast<double>(range) * range;
    std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin();
    while (it != mEntities.end())
    {
        if (*it != NULL && (includeRemoved || !(*it)->mRemoved) && (*it)->GetType() == type
            && (*it)->distanceToSqr(x, y, z) <= rangeSquared)
        {
            ++matches;
        }
        ++it;
    }
    return matches;
}

bool Level::mayPlace(Block* block, const BlockPos& pos, bool replaceable,
                     const Direction* direction, boost::shared_ptr<Entity> entity)
{
    const BlockState* oldState = getBlockState(pos);
    if (!replaceable && oldState != NULL && !oldState->getMaterial()->isReplaceable())
    {
        return false;
    }
    const AABB* shape = block->getShape(block->defaultBlockState(), this, pos);
    return (shape == NULL || isUnobstructed(shape->move(pos), entity)) && block->mayPlace(this, pos, direction);
}

boost::shared_ptr<Explosion> Level::explode(boost::shared_ptr<Entity> entity, double x, double y, double z, float radius,
                                            bool fire, bool smoke, bool blockInteraction, bool particles,
                                            bool sounds, float damage)
{
    boost::shared_ptr<Explosion> explosion(new Explosion(this, entity, x, y, z, radius, fire,
                                                          blockInteraction, false, !smoke, !sounds,
                                                          damage, false));
    explosion->explode();
    explosion->finalizeExplosion(particles, NULL);
    return explosion;
}

double Level::getSeenPercent(Vec3* source, AABB* box)
{
    double xStep = 1.0 / ((box->mMaxX - box->mMinX) * 2.0 + 1.0);
    double yStep = 1.0 / ((box->mMaxY - box->mMinY) * 2.0 + 1.0);
    double zStep = 1.0 / ((box->mMaxZ - box->mMinZ) * 2.0 + 1.0);
    int visible = 0;
    int total = 0;
    for (double x = 0.0; x <= 1.0; x += xStep)
    {
        for (double y = 0.0; y <= 1.0; y += yStep)
        {
            for (double z = 0.0; z <= 1.0; z += zStep)
            {
                Vec3* target = Vec3::newTemp(box->mMinX + (box->mMaxX - box->mMinX) * x,
                                             box->mMinY + (box->mMaxY - box->mMinY) * y,
                                             box->mMinZ + (box->mMaxZ - box->mMinZ) * z);
                if (clip(target, source) == NULL)
                {
                    ++visible;
                }
                ++total;
            }
        }
    }
    return total == 0 ? 0.0 : static_cast<double>(visible) / total;
}

bool Level::extinguishFire(boost::shared_ptr<Player> player, const BlockPos& pos,
                           const Direction* direction)
{
    BlockPos target = pos.relative(direction);
    if (getBlock(target) != Blocks::FIRE)
    {
        return false;
    }
    levelEvent(player, 1009, target, 0);
    return setBlock(target, Blocks::AIR->defaultBlockState(), 11, false);
}

int Level::getSkyColor(boost::shared_ptr<Entity> entity, float partialTick)
{
    boost::shared_ptr<Entity> source = entity;
    if (source == NULL)
    {
        return 0;
    }
    BlockPos pos(static_cast<int>(std::floor(source->mX)), static_cast<int>(std::floor(source->mY)),
                 static_cast<int>(std::floor(source->mZ)));
    int color = static_cast<int>(getBiome(pos)->getSkyColor(0.5f));
    float brightness = 1.0f - getSkyDarken(partialTick) / 15.0f;
    int red = static_cast<int>(((color >> 16) & 255) * brightness);
    int green = static_cast<int>(((color >> 8) & 255) * brightness);
    int blue = static_cast<int>((color & 255) * brightness);
    return (red << 16) | (green << 8) | blue;
}

Vec3* Level::getFogColor(boost::shared_ptr<Entity>, float partialTick)
{
    return mDimension->getFogColor(getTimeOfDay(partialTick), partialTick);
}

int Level::calcSnowBlockDepth(const BlockPos& pos, int depth)
{
    const BlockState* state = getBlockState(pos);
    if (state->getBlock() != Blocks::snow_layer)
    {
        return 0;
    }
    int layers = state->getBlock()->convertBlockStateToLegacyData(state) + 1;
    return layers < depth ? layers : depth;
}

bool Level::canPlaceSnowLayer(const BlockPos& pos, bool checkNeighbors, bool allowExisting, int* depth)
{
    const BlockState* state = getBlockState(pos);
    int layers = state->getBlock() == Blocks::snow_layer
        ? state->getBlock()->convertBlockStateToLegacyData(state) + 1 : 0;
    if (depth != NULL)
    {
        *depth = layers;
    }
    if (layers != 0)
    {
        return allowExisting && layers < 8;
    }
    if (!state->getMaterial()->isReplaceable())
    {
        return false;
    }
    if (checkNeighbors && !isSolidBlockingBlock(pos.below()))
    {
        return false;
    }
    return Blocks::snow_layer->mayPlace(this, pos);
}

bool Level::tryToPlaceTopSnow(const BlockPos& pos, bool checkNeighbors, bool allowExisting)
{
    int depth = 0;
    if (!canPlaceSnowLayer(pos, checkNeighbors, allowExisting, &depth))
    {
        return false;
    }
    const BlockState* state = Blocks::snow_layer->getBlockState(depth < 7 ? depth : 7);
    return setBlock(pos, state, 2, false);
}

arrayWithLength<unsigned char> Level::getBlocksAndData(int x, int y, int z, int width, int height,
                                                        int depth, bool includeLight)
{
    arrayWithLength<unsigned char> data(static_cast<unsigned int>(width * height * depth * 5), true);
    unsigned int index = 0;
    for (int yy = 0; yy < height; ++yy)
        for (int zz = 0; zz < depth; ++zz)
            for (int xx = 0; xx < width; ++xx)
            {
                BlockPos pos(x + xx, y + yy, z + zz);
                int blockId = getBlockId(pos.x, pos.y, pos.z);
                data.data[index++] = static_cast<unsigned char>(blockId);
                data.data[index++] = static_cast<unsigned char>(blockId >> 8);
                data.data[index++] = static_cast<unsigned char>(getBlockData(pos.x, pos.y, pos.z));
                data.data[index++] = static_cast<unsigned char>(getExtraBlockId(pos.x, pos.y, pos.z));
                data.data[index++] = includeLight ? static_cast<unsigned char>(getRawBrightness(pos)) : 0;
            }
    return data;
}

void Level::setBlocksAndData(int x, int y, int z, int width, int height, int depth,
                             arrayWithLength<unsigned char> data, bool)
{
    unsigned int index = 0;
    for (int yy = 0; yy < height; ++yy)
        for (int zz = 0; zz < depth; ++zz)
            for (int xx = 0; xx < width && index + 4 < data.length; ++xx)
            {
                int blockId = data.data[index] | (data.data[index + 1] << 8);
                int blockData = data.data[index + 2];
                setBlockAndData(x + xx, y + yy, z + zz, blockId, blockData, 2);
                index += 5;
            }
    setBlocksDirty(BlockPos(x, y, z), BlockPos(x + width, y + height, z + depth));
}

void Level::setFromRawData(int x, int z, arrayWithLength<unsigned char> data)
{
    LevelChunk* chunk = getChunk(x, z);
    if (chunk != NULL)
    {
        chunk->setFromRawData(data);
        setBlocksDirty(BlockPos(x * 16, 0, z * 16), BlockPos(x * 16 + 16, 256, z * 16 + 16));
    }
}

LevelChunk* Level::getCacheChunkDirect(int x, int z)
{
    return mChunkSource->getChunkIfLoaded(x, z);
}

bool Level::isChunkPostPostProcessed(int x, int z)
{
    LevelChunk* chunk = mChunkSource->getChunkIfLoaded(x, z);
    return chunk != NULL && !chunk->isEmpty() && chunk->postProcessAt(x, z);
}

bool Level::isChunkFinalised(int x, int z)
{
    LevelChunk* chunk = mChunkSource->getChunkIfLoaded(x, z);
    return chunk != NULL && chunk->isLoaded() && chunk->isLightPopulated();
}

void Level::refreshLevelInfoFromLevelGen()
{
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options != NULL)
    {
        mLevelData->setCloudHeight(options->getCloudHeight());
        mLevelData->setDayTime(options->getDayTime());
        mFogDistance = options->getFogDistance();
    }
}

void Level::refreshFromLevelGen()
{
    ConsoleSaveFile* saveFile = mLevelStorage->getSaveFile();
    if (saveFile != NULL)
    {
        saveFile->refreshFromLevelgen();
    }
    refreshLevelInfoFromLevelGen();
}

bool Level::canCreateMore(unsigned long long type, ESPAWN_TYPE spawnType)
{
    unsigned int count = 0;
    unsigned int matches = 0;
    countInstanceOf(type, false, &count, &matches);
    int limit = spawnType == 0 ? 70 : (spawnType == 1 ? 15 : 5);
    return static_cast<int>(matches) < limit;
}

double Level::getSpecialMultiplier(int type)
{
    double moon = getMoonBrightness();
    double difficulty = static_cast<double>(getDifficulty()->getId());
    if (type == 0)
    {
        return 1.0 + moon * difficulty * 0.25;
    }
    if (type == 1)
    {
        return 1.0 + getThunderLevel(1.0f) * 0.5;
    }
    return 1.0;
}

void Level::removeAllEntities(bool players, bool mobs, bool items, bool vehicles,
                              int x, int y, int z, int radius)
{
    double radiusSquared = static_cast<double>(radius) * radius;
    std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin();
    while (it != mEntities.end())
    {
        boost::shared_ptr<Entity> entity = *it;
        bool inRange = radius < 0 || entity->distanceToSqr(x, y, z) <= radiusSquared;
        if (inRange && (players || mobs || items || vehicles))
        {
            entity->remove();
            it = mEntities.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Level::updateNeighbourForOutputSignal(const BlockPos& pos, Block* block)
{
    std::vector<const Direction*>::iterator direction = Direction::VALUES.begin();
    while (direction != Direction::VALUES.end())
    {
        BlockPos neighbour = pos.relative(*direction);
        neighborChanged(neighbour, block, pos);
        ++direction;
    }
}
