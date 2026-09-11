#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/EntityBlock.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/world/DimensionType.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/core/CMinecraftApp.hpp"
#include <cstring>

unsigned int TlsAlloc();
bool TlsSetValue(unsigned int, void*);
void* TlsGetValue(unsigned int);

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

extern MinecraftServer* gMinecraftServer;
extern CMinecraftApp* g_pApp;

static unsigned int sLevelChunkThreadStorage = TlsAlloc();
static OSFastMutex sLevelChunkLightingMutex;
static OSFastMutex sLevelChunkEntityMutex;
static OSFastMutex sLevelChunkPostProcessMutex;

LevelChunk::LevelChunk(Level* level, int x, int z)
    : mEntitySectionCount(16), mElements(NULL), level(NULL), mSkyLight(NULL), mSkyLightUpper(NULL), mBlockLight(NULL),
      mBlockLightUpper(NULL), mEntitySections(NULL)
{
    init(level, x, z);
    mSkyLightUpper = new SparseLightStorage(true, true);
    mBlockLightUpper = new SparseLightStorage(false, true);
    mElements = new ElementStorage(level->mIsClientSide);
}
LevelChunk::LevelChunk(Level* level, ChunkPrimer* primer, int x, int z)
    : mEntitySectionCount(16), mElements(NULL), level(NULL), mSkyLight(NULL), mSkyLightUpper(NULL), mBlockLight(NULL),
      mBlockLightUpper(NULL), mEntitySections(NULL)
{
    init(level, x, z);
    arrayWithLength<unsigned short> elements = primer->getElementIds();
    if (elements.data != NULL)
    {
        mElements = new ElementStorage(level->mIsClientSide, elements, 4);
        mSkyLight = new SparseLightStorage(true);
        mBlockLight = new SparseLightStorage(false);
    }
    else
    {
        mElements = new ElementStorage(level->mIsClientSide);
    }
    mSkyLightUpper = new SparseLightStorage(true, true);
    mBlockLightUpper = new SparseLightStorage(false, true);
}
LevelChunk::LevelChunk(Level* level, int x, int z, LevelChunk* source)
    : mEntitySectionCount(16), mElements(NULL), level(NULL), mSkyLight(NULL), mSkyLightUpper(NULL), mBlockLight(NULL),
      mBlockLightUpper(NULL), mEntitySections(NULL)
{
    init(level, x, z);
    mElements = new ElementStorage(level->mIsClientSide, source->mElements);
    mSkyLight = new SparseLightStorage(source->mSkyLight);
    mSkyLightUpper = new SparseLightStorage(source->mSkyLightUpper);
    mBlockLight = new SparseLightStorage(source->mBlockLight);
    mBlockLightUpper = new SparseLightStorage(source->mBlockLightUpper);
    delete[] mBiomes.data;
    mBiomes = source->mBiomes;
    mOwnsBiomes = true;
}
LevelChunk::~LevelChunk()
{
    delete mElements;
    delete mSkyLight;
    delete mSkyLightUpper;
    delete mBlockLight;
    delete mBlockLightUpper;
    if (!mOwnsBiomes)
        delete[] mBiomes.data;
    delete[] mHeightmap.data;
    if (mEntitySections != NULL)
    {
        for (int i = 0; i < mEntitySectionCount; ++i)
            delete mEntitySections[i];
        delete[] mEntitySections;
    }
}
int LevelChunk::BlockChange::GetPackedPos() const
{
    return mPackedPos;
}
unsigned int LevelChunk::BlockChange::GetElement() const
{
    return (unsigned int)mElement;
}
BlockPos LevelChunk::BlockChange::GetPos() const
{
    return LevelChunk::UnpackBlockPos((short)mPackedPos);
}
const BlockState* LevelChunk::BlockChange::GetState() const
{
    return Block::getStateByElementId((unsigned short)(mElement & 255));
}
const BlockState* LevelChunk::BlockChange::GetExtraState() const
{
    return Block::getStateByElementId((unsigned short)((mElement >> 16) & 255));
}
LevelChunk::ThreadStorage::ThreadStorage() : elementStorage(NULL), extraElementStorage(NULL), tempLightingStorage(NULL)
{
}

bool LevelChunk::isReady()
{
    return mLightPopulated && (mChunkFlags & 0x400) != 0 && mPostProcess;
}
bool LevelChunk::isLoaded()
{
    return mLoaded;
}
bool LevelChunk::isLightPopulated()
{
    return mLightPopulated;
}
int LevelChunk::getBlockId(int x, int y, int z)
{
    return (int)(mElements->get(x, y, z, NULL) >> 4);
}
unsigned int LevelChunk::getLowestHeightmap()
{
    return mLowestHeightmap;
}
unsigned short LevelChunk::getElement(int x, int y, int z)
{
    return (unsigned short)mElements->get(x, y, z, NULL);
}
bool LevelChunk::sectionContainsBlockId(int x, int y, int z, int blockId)
{
    return mElements->sectionContainsElementData(x, y, z, (unsigned short)((blockId & 0xfff) << 4), 0x7ff0);
}
int LevelChunk::getExtraBlockId(int x, int y, int z)
{
    return (int)(mElements->getExtraElement(x, y, z) >> 4);
}
void LevelChunk::getBlockAndExtraId(int x, int y, int z, int* blockId, int* extraBlockId)
{
    unsigned short element = 0;
    unsigned short extra = 0;
    mElements->getElementAndExtraElement(x, y, z, &element, &extra);
    *blockId = element >> 4;
    *extraBlockId = extra >> 4;
}
unsigned int LevelChunk::getData(int x, int y, int z)
{
    return mElements->get(x, y, z, NULL) & 15;
}
unsigned int LevelChunk::getExtraBlockData(int x, int y, int z)
{
    return mElements->getExtraElement(x, y, z) & 15;
}
unsigned short LevelChunk::getElement(int x, int y, int z, bool* present)
{
    return (unsigned short)mElements->get(x, y, z, present);
}
int LevelChunk::getHighestSectionPosition()
{
    for (int section = 15; section >= 0; --section)
    {
        int highest = 0;
        if (!mElements->isRenderChunkEmpty(section * 16, &highest))
            return section * 16 + 15;
    }
    return 0;
}
BlockPos LevelChunk::getTopRainBlock(const BlockPos& pos)
{
    return BlockPos(pos.x, getHeightmap(pos), pos.z);
}
void LevelChunk::updateBiomeFlags(int x, int z)
{
    unsigned int index = ((x >> 1) | (z << 3)) & 127;
    mBiomeFlags[index] |= (unsigned char)(2 << ((x & 1) << 2));
}
bool LevelChunk::biomeHasRain(int x, int z)
{
    updateBiomeFlags(x, z);
    return (mBiomeFlags[((x >> 1) | (z << 3)) & 127] & (8 << ((x & 1) << 2))) != 0;
}
bool LevelChunk::biomeHasSnow(int x, int z)
{
    updateBiomeFlags(x, z);
    return (mBiomeFlags[((x >> 1) | (z << 3)) & 127] & (4 << ((x & 1) << 2))) != 0;
}
long long LevelChunk::getInhabitedTime()
{
    return mInhabitedTime;
}
bool LevelChunk::isTerrainPopulated()
{
    return (mChunkFlags & 0x400) != 0;
}
bool LevelChunk::isRenderChunkEmpty(int section)
{
    int highest = 0;
    return mElements->isRenderChunkEmpty(section, &highest);
}
void LevelChunk::getBlockData(arrayWithLength<unsigned char> data)
{
    arrayWithLength<unsigned char> extra;
    mElements->getData(data, extra, 0, 64);
}
void LevelChunk::setAllElements(arrayWithLength<unsigned char> elements, arrayWithLength<unsigned char> data)
{
    mElements->setData(elements, data);
}
void LevelChunk::setAllElements(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> data)
{
    mElements->setData(elements, data);
}
void LevelChunk::getAllElements(arrayWithLength<unsigned char> elements, arrayWithLength<unsigned char> data)
{
    mElements->getData(elements, data, 0, 64);
}
void LevelChunk::getElementRange(arrayWithLength<unsigned short> elements, int start, int end)
{
    mElements->getData(elements, start, end);
}
void LevelChunk::getElementRange(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> data,
                                 int start, int end)
{
    if (mElements != NULL)
        mElements->getData(elements, data, start, end);
}
int LevelChunk::getRawDataMaxBytes()
{
    return ElementStorage::getRawDataMaxBytes() + (int)SparseLightStorage::getRawDataMaxBytes() * 4 + 0x10c;
}
arrayWithLength<unsigned char> LevelChunk::getRawData()
{
    arrayWithLength<unsigned char> data(getRawDataMaxBytes(), false);
    data.length = mElements->getRawData(data.data);
    return data;
}
void LevelChunk::setFromRawData(arrayWithLength<unsigned char> data)
{
    unsigned int elementBytes = *(unsigned int*)data.data;
    mElements->setFromRawData(data.data + 12, elementBytes);
    unsigned char* cursor = data.data + 12 + elementBytes;
    unsigned short blockLightBytes = *(unsigned short*)(data.data + 4);
    if (mBlockLight == NULL)
        mBlockLight = new SparseLightStorage(false);
    mBlockLight->setFromRawData(cursor, blockLightBytes);
    cursor += blockLightBytes;
    unsigned short blockLightUpperBytes = *(unsigned short*)(data.data + 6);
    if (mBlockLightUpper == NULL)
        mBlockLightUpper = new SparseLightStorage(false, true);
    mBlockLightUpper->setFromRawData(cursor, blockLightUpperBytes);
    cursor += blockLightUpperBytes;
    unsigned short skyLightBytes = *(unsigned short*)(data.data + 8);
    if (mSkyLight == NULL)
        mSkyLight = new SparseLightStorage(true);
    mSkyLight->setFromRawData(cursor, skyLightBytes);
    cursor += skyLightBytes;
    unsigned short skyLightUpperBytes = *(unsigned short*)(data.data + 10);
    if (mSkyLightUpper == NULL)
        mSkyLightUpper = new SparseLightStorage(true, true);
    mSkyLightUpper->setFromRawData(cursor, skyLightUpperBytes);
    cursor += skyLightUpperBytes;
    std::memcpy(mBiomes.data, cursor, 0x100);
    recalcHeightmapOnly();
    for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
        it->second->clearCache();
    mChunkFlags = 0x400;
    mPostProcess = true;
}
void LevelChunk::getDataData(arrayWithLength<unsigned char> data)
{
    mElements->getData(data, data, 0, 256);
}
void LevelChunk::getSkyLightData(arrayWithLength<unsigned char> data)
{
    if (mSkyLight != NULL)
        mSkyLight->getData(data, 0);
}
void LevelChunk::getBlockLightData(arrayWithLength<unsigned char> data)
{
    if (mBlockLight != NULL)
        mBlockLight->getData(data, 0);
}
void LevelChunk::setSkyLightData(arrayWithLength<unsigned char> data)
{
    if (mSkyLight == NULL)
        mSkyLight = new SparseLightStorage(false);
    mSkyLight->setData(data, 0);
}
void LevelChunk::setBlockLightData(arrayWithLength<unsigned char> data)
{
    if (mBlockLight == NULL)
        mBlockLight = new SparseLightStorage(false);
    mBlockLight->setData(data, 0);
}
void LevelChunk::setSkyLightDataAllBright()
{
    if (mSkyLight == NULL)
        mSkyLight = new SparseLightStorage(true);
    else
        mSkyLight->setAllBright();
}
bool LevelChunk::isLowerBlockLightStorageCompressed()
{
    return mBlockLight != NULL && mBlockLight->isCompressed();
}
void LevelChunk::writeCompressedElements(DataOutputStream* stream)
{
    mElements->write(stream);
}
void LevelChunk::readCompressedElements(DataInputStream* stream)
{
    mElements->read(stream);
}
void LevelChunk::readCompressedElementsFromBlockAndData(DataInputStream* stream)
{
    CompressedBlockStorage blocks;
    CompressedBlockStorage extraBlocks;
    SparseDataStorage data;
    SparseDataStorage extraData;
    blocks.read(stream);
    extraBlocks.read(stream);
    data.read(stream);
    extraData.read(stream);
    delete mElements;
    mElements = new ElementStorage(false, &blocks, &extraBlocks, &data, &extraData);
}
void LevelChunk::writeCompressedSkyLightData(DataOutputStream* stream)
{
    if (mSkyLight != NULL)
        mSkyLight->write(stream);
}
void LevelChunk::writeCompressedBlockLightData(DataOutputStream* stream)
{
    if (mBlockLight != NULL)
        mBlockLight->write(stream);
}
unsigned char* LevelChunk::GetTempLightingStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage));
    return storage == NULL ? NULL : storage->tempLightingStorage;
}
void LevelChunk::readCompressedSkyLightData(DataInputStream* stream)
{
    if (mSkyLight == NULL)
        mSkyLight = new SparseLightStorage(true);
    mSkyLight->read(stream, GetTempLightingStorage());
}
void LevelChunk::readCompressedBlockLightData(DataInputStream* stream)
{
    if (mBlockLight == NULL)
        mBlockLight = new SparseLightStorage(false);
    mBlockLight->read(stream, GetTempLightingStorage());
}
unsigned int LevelChunk::getSparseBlockAndDataMemoryAllocated()
{
    return mElements == NULL ? 0 : mElements->getAllocatedBytes();
}
bool LevelChunk::getHasGapsToCheck()
{
    return mHasGapsToCheck;
}
unsigned int LevelChunk::getBiomePresentFlag()
{
    return mBiomePresent;
}
void LevelChunk::reSyncLighting()
{
    EnterCriticalSection(&sLevelChunkLightingMutex);
    if (!isEmpty())
    {
        const DimensionType* dimensionType = level->mDimension->getType();
        ServerLevel* otherLevel = gMinecraftServer->getLevel(dimensionType->getId());
        ChunkSource* chunkSource = otherLevel->getChunkSource();
        LevelChunk* otherChunk = chunkSource->getChunkLoadedOrUnloaded(x, z);
        if (!otherChunk->isEmpty())
        {
            GameRenderer::AddForDelete(mSkyLight);
            mSkyLight = new SparseLightStorage(otherChunk->mSkyLight);
            GameRenderer::FinishedReassigning();
            GameRenderer::AddForDelete(mBlockLight);
            mBlockLight = new SparseLightStorage(otherChunk->mBlockLight);
            GameRenderer::FinishedReassigning();
            GameRenderer::AddForDelete(mSkyLightUpper);
            mSkyLightUpper = new SparseLightStorage(otherChunk->mSkyLightUpper);
            GameRenderer::FinishedReassigning();
            GameRenderer::AddForDelete(mBlockLightUpper);
            mBlockLightUpper = new SparseLightStorage(otherChunk->mBlockLightUpper);
            GameRenderer::FinishedReassigning();
            GameRenderer::AddForDelete(mElements);
            mElements = new ElementStorage(level->mIsClientSide, otherChunk->mElements);
            GameRenderer::FinishedReassigning();
            level->setBlocksDirty(BlockPos(x * 16 - 1, 0, z * 16 - 1), BlockPos(x * 16 + 16, 256, z * 16 + 16));
            LeaveCriticalSection(&sLevelChunkLightingMutex);
            return;
        }
    }
    LeaveCriticalSection(&sLevelChunkLightingMutex);
}
bool LevelChunk::reSyncLightingNearby()
{
    reSyncLighting();
    return true;
}
void LevelChunk::dropLighting()
{
    delete mSkyLight;
    delete mSkyLightUpper;
    delete mBlockLight;
    delete mBlockLightUpper;
    mSkyLight = NULL;
    mSkyLightUpper = NULL;
    mBlockLight = NULL;
    mBlockLightUpper = NULL;
}
void LevelChunk::SetInvalid(bool invalid)
{
    mInvalid = invalid;
    if (invalid)
    {
        EnterCriticalSection(&sLevelChunkEntityMutex);
        for (int i = 0; i < mEntitySectionCount; ++i)
            mEntitySections[i]->clear();
        LeaveCriticalSection(&sLevelChunkEntityMutex);
        EnterCriticalSection(&sLevelChunkPostProcessMutex);
        for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
            level->markForRemoval(it->second);
        mBlockEntityMap.clear();
        LeaveCriticalSection(&sLevelChunkPostProcessMutex);
        delete mPendingBlockEntityTag;
        mPendingBlockEntityTag = NULL;
        mUnloaded = true;
    }
}
bool LevelChunk::isInvalid() const
{
    return mInvalid;
}
void LevelChunk::getEntityReadLock()
{
    EnterCriticalSection(&sLevelChunkEntityMutex);
}
void LevelChunk::releaseEntityReadLock()
{
    LeaveCriticalSection(&sLevelChunkEntityMutex);
}
bool LevelChunk::hasLightingBeenDropped()
{
    return mSkyLight == NULL && mBlockLight == NULL;
}
void LevelChunk::init(Level* level, int x, int z)
{
    this->level = level;
    this->x = x;
    this->z = z;
    mBiomes = arrayWithLength<unsigned char>(256, false);
    for (int i = 0; i < 256; ++i)
        mBiomes.data[i] = 255;
    mHeightmap = arrayWithLength<unsigned char>(256, true);
    for (int i = 0; i < 256; ++i)
        mPrecipitationHeightmap[i] = 255;
    for (int i = 0; i < 128; ++i)
        mBiomeFlags[i] = 0;
    mPrecipitationHeightmapMinimum = 0;
    mMinHeight = 0;
    mMaxHeight = 0;
    mHasGapsToCheck = false;
    mBiomePresent = 0;
    mEntitySections = new std::vector<boost::shared_ptr<Entity>>*[mEntitySectionCount];
    for (int i = 0; i < mEntitySectionCount; ++i)
        mEntitySections[i] = new std::vector<boost::shared_ptr<Entity>>();
    mChunkFlags = 0;
    mChunkFlagsPadding = 0;
    mPostProcessIndex = 0;
    mPostProcess = false;
    mLightPopulated = false;
    mUnsaved = false;
    mUnloaded = false;
    mLastSaveHadEntities = false;
    mLoaded = false;
    mEmissiveMaxHeight = 0;
    mEntityCount = 0;
    mLightCheckIndex = 0;
    mQueuedLightChecks = 0;
    mLastSaveTime = 0;
    mLowestHeightmap = 0;
    mInhabitedTime = 0;
    mInvalid = false;
    mPendingBlockEntityTag = NULL;
    mWillBeSaved = false;
    mCheckAllLight = 0;
    mChunkVersion = 0;
    mPostProcessFlags = 0;
    mPendingBlockEntitiesChanged = false;
    mOwnsBiomes = false;
}
bool LevelChunk::isAt(int x, int z)
{
    return x == this->x && z == this->z;
}
int LevelChunk::getHeightmap(const BlockPos& pos)
{
    return getHeightmap(pos.x & 15, pos.z & 15);
}
unsigned char LevelChunk::getHeightmap(int x, int z)
{
    return mHeightmap.data[(z << 4) | x];
}
void LevelChunk::recalcBlockLights() {}
int LevelChunk::getHighestNonEmptyY()
{
    return mElements->getHighestNonEmptyY();
}
unsigned short* LevelChunk::GetElementStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage));
    return storage == NULL ? NULL : storage->elementStorage;
}
unsigned short* LevelChunk::GetExtraElementStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage));
    return storage == NULL ? NULL : storage->extraElementStorage;
}
void LevelChunk::setUnsaved(bool unsaved)
{
    mUnsaved = unsaved;
}
void LevelChunk::recalcHeightmapOnly()
{
    mLowestHeightmap = 256;
    for (int localZ = 0; localZ < 16; ++localZ)
        for (int localX = 0; localX < 16; ++localX)
        {
            int height = getHighestNonEmptyY() + 16;
            if (height > 256)
                height = 256;
            while (height > 0 && getBlockState(localX, height - 1, localZ)->getLightBlock() == 0)
                --height;
            mHeightmap.data[(localZ << 4) | localX] = (unsigned char)height;
            if ((unsigned int)height < mLowestHeightmap)
                mLowestHeightmap = height;
        }
    recalcMinAndMaxHeights();
}
void LevelChunk::lightGaps(int x, int z)
{
    int index = (x >> 1) | (z << 3);
    mBiomeFlags[index] |= (unsigned char)(1 << ((x & 1) << 2));
    mHasGapsToCheck = true;
}
void LevelChunk::recalcHeightmap()
{
    recalcHeightmapOnly();
    mHasGapsToCheck = false;
    for (int i = 0; i < 128; ++i)
        mBiomeFlags[i] = 0;
}
Block* LevelChunk::getChunkBlock(int x, int y, int z)
{
    return Block::byId(getBlockId(x, y, z));
}
void LevelChunk::lightLava()
{
    int top = getHighestSectionPosition();
    for (int localX = 0; localX < 16; ++localX)
        for (int localZ = 0; localZ < 16; ++localZ)
            for (int y = 0; y <= top; ++y)
            {
                const BlockState* state = getBlockState(localX, y, localZ);
                if (state->getBlock()->getLightEmission(state) > 0)
                    level->checkLight(BlockPos(x * 16 + localX, y, z * 16 + localZ), false, false);
            }
}
void LevelChunk::lightGap(int x, int z, int fromY, int toY)
{
    if (fromY < toY && level->hasChunksAt(BlockPos(x, 128, z), 16))
    {
        for (int y = fromY; y < toY; ++y)
            level->checkLight(LightLayer::SKY, BlockPos(x, y, z), false, false);
        setUnsaved(true);
    }
}
void LevelChunk::lightGap(int x, int y, int z)
{
    int height = level->getHeightmapPos(BlockPos(x, 0, z)).y;
    if (height < y)
        lightGap(x, z, height, y + 1);
    else if (y < height)
        lightGap(x, z, y, height + 1);
}
void LevelChunk::recheckGaps(bool onlyOne)
{
    if (!isEmpty())
    {
        int minPos = (level->mDimension->getXZSize() * -16) >> 1;
        int maxPos = ((level->mDimension->getXZSize() << 4) >> 1) - 1;
        if (level->hasChunksAt(BlockPos(x * 16 + 8, 128, z * 16 + 8), 16))
        {
            for (int localX = 0; localX < 16; ++localX)
            {
                for (int localZ = 0; localZ < 16; ++localZ)
                {
                    int index = (localX >> 1) | (localZ << 3);
                    unsigned char mask = (unsigned char)(1 << ((localX & 1) << 2));
                    unsigned char flags = mBiomeFlags[index];
                    if (onlyOne || (flags & mask) != 0)
                    {
                        mBiomeFlags[index] = (unsigned char)(flags & ~mask);
                        unsigned char height = getHeightmap(localX, localZ);
                        int worldX = x * 16 + localX;
                        int worldZ = z * 16 + localZ;
                        BlockPos pos(worldX, 0, worldZ);
                        int minHeight = level->getHeightmapPos(pos).y;
                        int westX = worldX - 1;
                        if (minPos <= westX)
                        {
                            int westHeight = level->getHeightmapPos(pos.west()).y;
                            if (westHeight < minHeight)
                                minHeight = westHeight;
                        }
                        int eastX = worldX + 1;
                        if (eastX <= maxPos)
                        {
                            int eastHeight = level->getHeightmapPos(pos.east()).y;
                            if (eastHeight < minHeight)
                                minHeight = eastHeight;
                        }
                        int northZ = worldZ - 1;
                        if (minPos <= northZ)
                        {
                            int northHeight = level->getHeightmapPos(pos.north()).y;
                            if (northHeight < minHeight)
                                minHeight = northHeight;
                        }
                        int southZ = worldZ + 1;
                        if (southZ <= maxPos)
                        {
                            int southHeight = level->getHeightmapPos(pos.south()).y;
                            if (southHeight < minHeight)
                                minHeight = southHeight;
                        }
                        lightGap(worldX, worldZ, minHeight);
                        if (!onlyOne)
                        {
                            if (minPos <= westX)
                                lightGap(westX, worldZ, height);
                            if (eastX <= maxPos)
                                lightGap(eastX, worldZ, height);
                            if (minPos <= northZ)
                                lightGap(worldX, northZ, height);
                            if (southZ <= maxPos)
                                lightGap(worldX, southZ, height);
                        }
                        mHasGapsToCheck = false;
                    }
                }
            }
        }
    }
}
void LevelChunk::recalcHeight(int localX, int y, int localZ)
{
    int oldHeight = getHeightmap(localX, localZ);
    int height = y > oldHeight ? y : oldHeight;
    while (height > 0 && getBlockState(localX, height - 1, localZ)->getLightBlock() == 0)
        --height;
    if (height != oldHeight)
    {
        mHeightmap.data[(localZ << 4) | localX] = (unsigned char)height;
        lightGap(x * 16 + localX, z * 16 + localZ, height < oldHeight ? height : oldHeight,
                 height > oldHeight ? height : oldHeight);
        recalcMinAndMaxHeights();
        setUnsaved(true);
    }
}
int LevelChunk::getBlockLightBlock(const BlockPos& pos)
{
    int block = getBlock(pos)->defaultBlockState()->getLightBlock();
    int extra = getExtraBlock(pos)->defaultBlockState()->getLightBlock();
    return block > extra ? block : extra;
}
int LevelChunk::getChunkBlockLightBlock(int x, int y, int z)
{
    return getBlockLightBlock(BlockPos(this->x * 16 + x, y, this->z * 16 + z));
}
Block* LevelChunk::getBlock(const BlockPos& pos)
{
    return Block::byId(getBlockId(pos.x & 15, pos.y, pos.z & 15));
}
Block* LevelChunk::getExtraBlock(const BlockPos& pos)
{
    return Block::byId(getExtraBlockId(pos.x & 15, pos.y, pos.z & 15));
}
const BlockState* LevelChunk::getBlockState(int x, int y, int z)
{
    return Block::getStateByElementId((unsigned short)mElements->get(x, y, z, NULL));
}
const BlockState* LevelChunk::getBlockState(const BlockPos& pos)
{
    return getBlockState(pos.x & 15, pos.y, pos.z & 15);
}
const BlockState* LevelChunk::getExtraBlockState(const BlockPos& pos)
{
    return Block::getStateByElementId((unsigned short)mElements->getExtraElement(pos.x & 15, pos.y, pos.z & 15));
}
void LevelChunk::getBlockAndExtraId_NoLock(int x, int y, int z, int* blockId, int* extraBlockId)
{
    unsigned short element = 0;
    unsigned short extra = 0;
    mElements->getElementAndExtraElement_NoLock(x, y, z, &element, &extra);
    *blockId = element >> 4;
    *extraBlockId = extra >> 4;
}
void LevelChunk::getElementReadLock()
{
    mElements->getReadLockExternal();
}
void LevelChunk::releaseElementReadLock()
{
    mElements->releaseReadLockExternal();
}
unsigned int LevelChunk::getElementFrameLastUpdated()
{
    return mElements->getFrameLastUpdated();
}
unsigned short LevelChunk::setElement(int x, int y, int z, unsigned short element)
{
    unsigned short old = mElements->get(x, y, z, NULL);
    mElements->set(x, y, z, element);
    return old;
}
unsigned short LevelChunk::getChunkBlockData(int x, int y, int z)
{
    return mElements->get(x, y, z, NULL);
}
int LevelChunk::getData(const BlockPos& pos)
{
    return getData(pos.x & 15, pos.y, pos.z & 15);
}
bool LevelChunk::setData(int x, int y, int z, int data, int flags, bool* changed)
{
    unsigned short old = mElements->get(x, y, z, NULL);
    mElements->set(x, y, z, (unsigned short)((old & 0xfff0) | (data & 15)));
    if (changed != NULL)
        *changed = old != mElements->get(x, y, z, NULL);
    return old != mElements->get(x, y, z, NULL);
}
void LevelChunk::setBlock(const BlockPos& pos, const BlockState* state)
{
    internalSetBlock(false, pos, state);
}
void LevelChunk::setExtraBlock(const BlockPos& pos, const BlockState* state)
{
    internalSetBlock(true, pos, state);
}
bool LevelChunk::internalSetBlock(bool extra, const BlockPos& pos, const BlockState* state)
{
    if (state == NULL)
        return false;
    unsigned short value = (unsigned short)state->mElementId;
    unsigned short old = extra ? mElements->getExtraElement(pos.x & 15, pos.y, pos.z & 15)
                               : mElements->get(pos.x & 15, pos.y, pos.z & 15, NULL);
    if (extra)
        mElements->setExtraElement(pos.x & 15, pos.y, pos.z & 15, value);
    else
        mElements->set(pos.x & 15, pos.y, pos.z & 15, value);
    mUnsaved = true;
    return old != value;
}
bool LevelChunk::setBlockAndData(int x, int y, int z, int blockId, int data, bool update, bool extra)
{
    Block* block = Block::byId(blockId);
    if (block == NULL)
        return false;
    const BlockState* state = block->defaultBlockState();
    bool changed = internalSetBlock(extra, BlockPos(this->x * 16 + x, y, this->z * 16 + z), state);
    return changed;
}
void LevelChunk::setExtraElement(int x, int y, int z, unsigned short element)
{
    mElements->setExtraElement(x, y, z, element);
}
unsigned short LevelChunk::getExtraElement(int x, int y, int z)
{
    return mElements->getExtraElement(x, y, z);
}
void LevelChunk::getElementAndExtraElement(int x, int y, int z, unsigned short* element, unsigned short* extraElement)
{
    mElements->getElementAndExtraElement(x, y, z, element, extraElement);
}
int LevelChunk::getBrightness(LightLayer::variety layer, const BlockPos& pos)
{
    SparseLightStorage* storage = layer == LightLayer::SKY ? (pos.y < 128 ? mSkyLight : mSkyLightUpper)
                                                           : (pos.y < 128 ? mBlockLight : mBlockLightUpper);
    return storage == NULL ? 0 : storage->get(pos.x & 15, pos.y & 127, pos.z & 15);
}
void LevelChunk::getNeighbourBrightnesses(int* brightnesses, LightLayer::variety layer, int x, int y, int z)
{
    if (brightnesses == NULL)
        return;
    brightnesses[0] = getBrightness(layer, BlockPos(x - 1, y, z));
    brightnesses[1] = getBrightness(layer, BlockPos(x + 1, y, z));
    brightnesses[2] = getBrightness(layer, BlockPos(x, y - 1, z));
    brightnesses[3] = getBrightness(layer, BlockPos(x, y + 1, z));
}
bool LevelChunk::setBrightness(LightLayer::variety layer, const BlockPos& pos, int brightness)
{
    SparseLightStorage*& storage = layer == LightLayer::SKY ? (pos.y < 128 ? mSkyLight : mSkyLightUpper)
                                                            : (pos.y < 128 ? mBlockLight : mBlockLightUpper);
    if (storage == NULL)
        storage = new SparseLightStorage(layer == LightLayer::SKY);
    storage->set(pos.x & 15, pos.y & 127, pos.z & 15, brightness);
    mUnsaved = true;
    return true;
}
int LevelChunk::getRawBrightness(const BlockPos& pos, int skyDarken)
{
    int brightness = getBrightness(LightLayer::SKY, pos) - skyDarken;
    int block = getBrightness(LightLayer::BLOCK, pos);
    return brightness > block ? brightness : block;
}
void LevelChunk::addEntity(boost::shared_ptr<Entity> entity)
{
    mLastSaveHadEntities = true;
    int chunkX = (int)(entity->mX * 0.0625);
    if (entity->mX * 0.0625 < chunkX)
        --chunkX;
    int chunkZ = (int)(entity->mZ * 0.0625);
    if (entity->mZ * 0.0625 < chunkZ)
        --chunkZ;
    int section = (int)(entity->mY * 0.0625);
    if (entity->mY * 0.0625 < section)
        --section;
    if (section < 0)
        section = 0;
    if (section >= mEntitySectionCount)
        section = mEntitySectionCount - 1;
    entity->mAddedToChunk = true;
    entity->mChunkX = x;
    entity->mChunkY = section;
    entity->mChunkZ = z;
    EnterCriticalSection(&sLevelChunkEntityMutex);
    mEntitySections[section]->push_back(entity);
    LeaveCriticalSection(&sLevelChunkEntityMutex);
}
void LevelChunk::removeEntity(boost::shared_ptr<Entity> entity)
{
    removeEntity(entity, entity->mChunkY);
}
void LevelChunk::removeEntity(boost::shared_ptr<Entity> entity, int section)
{
    if (section < 0)
        section = 0;
    if (section >= mEntitySectionCount)
        section = mEntitySectionCount - 1;
    EnterCriticalSection(&sLevelChunkEntityMutex);
    std::vector<boost::shared_ptr<Entity>>& entities = *mEntitySections[section];
    for (std::vector<boost::shared_ptr<Entity>>::iterator it = entities.begin(); it != entities.end(); ++it)
    {
        if (it->get() == entity.get())
        {
            entities.erase(it);
            break;
        }
    }
    LeaveCriticalSection(&sLevelChunkEntityMutex);
}
bool LevelChunk::isSkyLit(const BlockPos& pos)
{
    return getBrightness(LightLayer::SKY, pos) > 0;
}
boost::shared_ptr<BlockEntity> LevelChunk::createBlockEntity(const BlockPos& pos)
{
    const BlockState* state = getBlockState(pos);
    Block* block = state->getBlock();
    if (!block->mBlockEntity)
        return boost::shared_ptr<BlockEntity>();
    EntityBlock* entityBlock = dynamic_cast<EntityBlock*>(block);
    return entityBlock->newBlockEntity(level, state->getBlock()->convertBlockStateToLegacyData(state));
}
boost::shared_ptr<BlockEntity> LevelChunk::getBlockEntity(const BlockPos& pos, EntityCreationType type)
{
    BlockEntityMap::iterator it = mBlockEntityMap.find(pos);
    if (it != mBlockEntityMap.end())
        return it->second;
    return boost::shared_ptr<BlockEntity>();
}
void LevelChunk::addBlockEntity(boost::shared_ptr<BlockEntity> entity)
{
    if (entity.get() == NULL)
        return;
    entity->setLevel(level);
    mBlockEntityMap[entity->getBlockPos()] = entity;
}
void LevelChunk::setBlockEntity(const BlockPos& pos, boost::shared_ptr<BlockEntity> entity)
{
    removeBlockEntity(pos);
    if (entity.get() != NULL)
    {
        entity->setLevel(level);
        mBlockEntityMap[pos] = entity;
    }
}
void LevelChunk::removeBlockEntity(const BlockPos& pos)
{
    mBlockEntityMap.erase(pos);
}
bool LevelChunk::load(bool full)
{
    mLoaded = true;
    mUnloaded = false;
    return true;
}
bool LevelChunk::unload(bool save, bool keepLoaded)
{
    if (save)
        mUnsaved = false;
    mUnloaded = !keepLoaded;
    mLoaded = keepLoaded;
    return true;
}
bool LevelChunk::containsPlayer()
{
    for (int section = 0; section < mEntitySectionCount; ++section)
        for (std::vector<boost::shared_ptr<Entity>>::const_iterator it = mEntitySections[section]->begin();
             it != mEntitySections[section]->end(); ++it)
            if ((*it)->GetType() == 0x0c800001ULL)
                return true;
    return false;
}
bool LevelChunk::isUnloaded()
{
    return mUnloaded;
}
void LevelChunk::markUnsaved()
{
    mUnsaved = true;
}
void LevelChunk::getEntities(boost::shared_ptr<Entity> except, const AABB* box,
                             std::vector<boost::shared_ptr<Entity>>& entities,
                             const Predicate<boost::shared_ptr<Entity>>* predicate)
{
    int first = (int)((box->mMinY - 2.0) * 0.0625);
    if ((box->mMinY - 2.0) * 0.0625 < first)
        --first;
    int last = (int)((box->mMaxY + 2.0) * 0.0625);
    if ((box->mMaxY + 2.0) * 0.0625 < last)
        --last;
    if (first < 0)
        first = 0;
    if (last >= mEntitySectionCount)
        last = mEntitySectionCount - 1;
    for (int section = first; section <= last; ++section)
    {
        std::vector<boost::shared_ptr<Entity>>& sectionEntities = *mEntitySections[section];
        for (std::vector<boost::shared_ptr<Entity>>::iterator it = sectionEntities.begin(); it != sectionEntities.end();
             ++it)
        {
            boost::shared_ptr<Entity> entity = *it;
            if (entity != except &&
                box->intersects(entity->mBoundingBox->mMinX, entity->mBoundingBox->mMinY, entity->mBoundingBox->mMinZ,
                                entity->mBoundingBox->mMaxX, entity->mBoundingBox->mMaxY,
                                entity->mBoundingBox->mMaxZ) &&
                (predicate == NULL || predicate->apply(entity)))
                entities.push_back(entity);
            std::vector<boost::weak_ptr<Entity>>* passengers = entity->getPassengers();
            if (passengers != NULL)
                for (std::vector<boost::weak_ptr<Entity>>::iterator passenger = passengers->begin();
                     passenger != passengers->end(); ++passenger)
                {
                    boost::shared_ptr<Entity> passengerEntity = passenger->lock();
                    if (passengerEntity && passengerEntity != except &&
                        box->intersects(passengerEntity->mBoundingBox->mMinX, passengerEntity->mBoundingBox->mMinY,
                                        passengerEntity->mBoundingBox->mMinZ, passengerEntity->mBoundingBox->mMaxX,
                                        passengerEntity->mBoundingBox->mMaxY, passengerEntity->mBoundingBox->mMaxZ) &&
                        (predicate == NULL || predicate->apply(passengerEntity)))
                        entities.push_back(passengerEntity);
                }
        }
    }
}
void LevelChunk::getEntitiesOfClass(const std::type_info& type, const AABB* box,
                                    std::vector<boost::shared_ptr<Entity>>& entities,
                                    const Predicate<boost::shared_ptr<Entity>>* predicate, bool exact)
{
    int first = (int)((box->mMinY - 2.0) * 0.0625);
    if ((box->mMinY - 2.0) * 0.0625 < first)
        --first;
    int last = (int)((box->mMaxY + 2.0) * 0.0625);
    if ((box->mMaxY + 2.0) * 0.0625 < last)
        --last;
    if (first < 0)
        first = 0;
    if (last >= mEntitySectionCount)
        last = mEntitySectionCount - 1;
    for (int section = first; section <= last; ++section)
    {
        std::vector<boost::shared_ptr<Entity>>& sectionEntities = *mEntitySections[section];
        for (std::vector<boost::shared_ptr<Entity>>::iterator it = sectionEntities.begin(); it != sectionEntities.end();
             ++it)
        {
            boost::shared_ptr<Entity> entity = *it;
            bool matches = exact ? typeid(*entity) == type : typeid(*entity) == type;
            if (matches &&
                box->intersects(entity->mBoundingBox->mMinX, entity->mBoundingBox->mMinY, entity->mBoundingBox->mMinZ,
                                entity->mBoundingBox->mMaxX, entity->mBoundingBox->mMaxY,
                                entity->mBoundingBox->mMaxZ) &&
                (predicate == NULL || predicate->apply(entity)))
                entities.push_back(entity);
        }
    }
}
int LevelChunk::countEntities()
{
    int count = 0;
    for (int section = 0; section < mEntitySectionCount; ++section)
        count += (int)mEntitySections[section]->size();
    return count;
}
bool LevelChunk::shouldSave(bool force)
{
    if (mUnloaded)
        return false;
    if (!force)
        return (mLastSaveHadEntities && level->getGameTime() >= mLastSaveTime + 600) || mUnsaved;
    return (mLastSaveHadEntities && level->getGameTime() != mLastSaveTime) || mUnsaved;
}
int LevelChunk::getBlocksAndData(arrayWithLength<unsigned char>* output, int x, int y, int z, int width, int height,
                                 int depth, int offset, bool includeLight)
{
    int next = offset + mElements->getDataRegion(*output, x, y, z, width, height, depth, offset);
    if (includeLight)
    {
        if (y < 128)
            next += mBlockLight->getDataRegion(*output, x, y, z, width, height < 128 ? height : 128, depth, next);
        if (height > 128)
            next +=
                mBlockLightUpper->getDataRegion(*output, x, y < 128 ? 0 : y - 128, z, width, height - 128, depth, next);
        if (y < 128)
            next += mSkyLight->getDataRegion(*output, x, y, z, width, height < 128 ? height : 128, depth, next);
        if (height > 128)
            next +=
                mSkyLightUpper->getDataRegion(*output, x, y < 128 ? 0 : y - 128, z, width, height - 128, depth, next);
    }
    return next;
}
void LevelChunk::blockUpdatedCallback(int x, int y, int z, void* data, int value)
{
    LevelChunk* chunk = static_cast<LevelChunk*>(data);
    chunk->level->checkLight(BlockPos(chunk->x * 16 + x, y + value, chunk->z * 16 + z), false, false);
}
int LevelChunk::setBlocksAndData(arrayWithLength<unsigned char> input, int x, int y, int z, int width, int height,
                                 int depth, int offset, bool includeLight)
{
    if (includeLight)
    {
        GameRenderer::AddForDelete(mSkyLight);
        mSkyLight = new SparseLightStorage(true, false);
        GameRenderer::FinishedReassigning();
        GameRenderer::AddForDelete(mBlockLight);
        mBlockLight = new SparseLightStorage(false, false);
        GameRenderer::FinishedReassigning();
    }
    GameRenderer::EnterDeleteThreadCS();
    offset += mElements->setDataRegion(input, x, y, z, width, height, depth, offset,
                                       includeLight ? NULL : blockUpdatedCallback, this, 0);
    recalcHeightmapOnly();
    GameRenderer::LeaveDeleteThreadCS();
    if (includeLight)
    {
        if (y < 128)
            offset += mBlockLight->setDataRegion(input, x, y, z, width, height < 128 ? height : 128, depth, offset);
        if (height > 128)
            offset +=
                mBlockLightUpper->setDataRegion(input, x, y < 128 ? 0 : y - 128, z, width, height - 128, depth, offset);
        if (y < 128)
            offset += mSkyLight->setDataRegion(input, x, y, z, width, height < 128 ? height : 128, depth, offset);
        if (height > 128)
            offset +=
                mSkyLightUpper->setDataRegion(input, x, y < 128 ? 0 : y - 128, z, width, height - 128, depth, offset);
        std::memcpy(mBiomes.data, input.data + offset, mBiomes.length);
        offset += mBiomes.length;
        recalcHeightmap();
        lightLava();
        recalcMinAndMaxHeights();
    }
    mChunkFlags = 0x400;
    mPostProcess = true;
    return offset;
}
Random LevelChunk::getRandom(long long seed)
{
    long long chunkSeed = level->getSeed() + x * x * 0x4c1906LL + x * 0x5ac0dbLL + z * z * 0x4307a7LL + z * 0x5f24fLL;
    return Random(chunkSeed ^ seed);
}
bool LevelChunk::isEmpty()
{
    return false;
}
void LevelChunk::staticCtor()
{
    InitializeCriticalSection(&sLevelChunkLightingMutex);
    InitializeCriticalSection(&sLevelChunkEntityMutex);
    InitializeCriticalSection(&sLevelChunkPostProcessMutex);
}
void LevelChunk::postProcess()
{
    mPostProcess = 1;
}
void LevelChunk::flagPostProcessComplete(short value)
{
    mChunkFlags |= (unsigned short)value;
    if ((mChunkFlags & 0x1e) == 0x1e)
    {
        PIXBeginNamedEvent(0.0f, "Compressing lighting blocks");
        mElements->removeCompressionHeadroom();
        if (!isLowerBlockLightStorageCompressed())
            compressLighting();
        PIXEndNamedEvent();
    }
    if (mChunkFlags == 0x3fe)
    {
        g_pApp->processSchematicsLighting(this);
        PIXBeginNamedEvent(0.0f, "Recheck gaps");
        if (level->mDimension->getType() != DimensionType::NETHER)
            recheckGaps(true);
        PIXEndNamedEvent();
        PIXBeginNamedEvent(0.0f, "Light lava (this)");
        lightLava();
        PIXEndNamedEvent();
        mChunkFlags |= 0x400;
    }
}
void LevelChunk::doPostProcess(ChunkSource* source, ChunkGenerator* generator)
{
    if ((mChunkFlags & 2) == 0)
    {
        postProcess();
        generator->postProcess(x, z);
        markUnsaved();
        mChunkFlags |= 2;
        flagPostProcessComplete(0);
        source->flagPostProcessComplete(4, x + 1, z);
        source->flagPostProcessComplete(0x10, x + 1, z + 1);
        source->flagPostProcessComplete(8, x, z + 1);
        source->flagPostProcessComplete(0x200, x - 1, z + 1);
        source->flagPostProcessComplete(0x100, x - 1, z);
        source->flagPostProcessComplete(0x80, x - 1, z - 1);
        source->flagPostProcessComplete(0x40, x, z - 1);
        source->flagPostProcessComplete(0x20, x + 1, z - 1);
    }
    else if (generator->postProcessLoadedChunk(this, x, z))
        markUnsaved();
}
void LevelChunk::checkPostProcess(ChunkSource* source, ChunkGenerator* generator)
{
    LevelChunk* north = source->getChunkIfLoaded(x, z - 1);
    LevelChunk* east = source->getChunkIfLoaded(x + 1, z);
    LevelChunk* south = source->getChunkIfLoaded(x, z + 1);
    LevelChunk* west = source->getChunkIfLoaded(x - 1, z);
    if (east != NULL && south != NULL && source->getChunkIfLoaded(x + 1, z + 1) != NULL)
        doPostProcess(source, generator);
    if (west != NULL && south != NULL && source->getChunkIfLoaded(x - 1, z + 1) != NULL)
        west->doPostProcess(source, generator);
    if (north != NULL)
    {
        if (east != NULL && source->getChunkIfLoaded(x + 1, z - 1) != NULL)
            north->doPostProcess(source, generator);
        if (west != NULL && source->getChunkIfLoaded(x - 1, z - 1) != NULL)
            source->getChunkIfLoaded(x - 1, z - 1)->doPostProcess(source, generator);
    }
}
void LevelChunk::checkChests(ChunkSource* source, int chunkX, int chunkZ)
{
    LevelChunk* chunk = source->getChunkLoadedOrUnloaded(chunkX, chunkZ);
    if (chunk == NULL || mChunkVersion >= 10)
        return;
    for (int localX = 0; localX < 16; ++localX)
        for (int localZ = 0; localZ < 16; ++localZ)
            for (int y = 0; y < 128; ++y)
                chunk->getBlockState(localX, y, localZ);
}
void LevelChunk::tick(bool update)
{
    if (mHasGapsToCheck && !update)
        recheckGaps(level->mIsClientSide);
    mLoaded = true;
    if (!mPostProcess && isTerrainPopulated())
        postProcess();
    while (!mPendingBlockEntities.empty())
    {
        BlockPos pos = mPendingBlockEntities.front();
        mPendingBlockEntities.pop_front();
        boost::shared_ptr<BlockEntity> entity = getBlockEntity(pos, IMMEDIATE);
        if (!entity)
        {
            entity = createBlockEntity(pos);
            if (entity)
                level->setBlockEntity(pos, entity);
        }
    }
}
ChunkPos LevelChunk::getPos()
{
    return ChunkPos(x, z);
}
bool LevelChunk::isYSpaceEmpty(int y0, int y1)
{
    return false;
}
void LevelChunk::reloadBiomes()
{
    BiomeSource* source = level->getBiomeSource();
    for (int localX = 0; localX < 16; ++localX)
        for (int localZ = 0; localZ < 16; ++localZ)
        {
            Biome* biome = source->getBiome(BlockPos(x * 16 + localX, 0, z * 16 + localZ));
            mBiomes.data[(localZ << 4) | localX] = (unsigned char)Biome::getId(biome);
        }
}
Biome* LevelChunk::getBiome(const BlockPos& pos, BiomeSource* source)
{
    return source == NULL ? NULL : source->getBiome(pos);
}
arrayWithLength<unsigned char> LevelChunk::getBiomes()
{
    return mBiomes;
}
void LevelChunk::updateBiomePresentFlags()
{
    if (mBiomes.data == NULL)
        return;
    for (unsigned int i = 0; i < mBiomes.length; ++i)
    {
        unsigned int biome = mBiomes.data[i];
        if (biome == 0)
            mBiomePresent |= 1;
        else if (biome == 6)
            mBiomePresent |= 8;
        else if (biome == 24)
            mBiomePresent |= 2;
        else if (biome >= 40 && biome < 48)
            mBiomePresent |= (biome & 1) == 0 ? 1 : 2;
    }
}
void LevelChunk::setBiomes(arrayWithLength<unsigned char> biomes)
{
    mBiomes = biomes;
    updateBiomePresentFlags();
}
void LevelChunk::setCheckAllLight()
{
    mCheckAllLight = 1;
}
bool LevelChunk::postProcessAt(int x, int z)
{
    PIXBeginNamedEvent(0.0f, "LevelChunk::postProcessAt %d,%d", x, z);
    MutableBlockPos pos(this->x * 16 + x, 0, this->z * 16 + z);
    int y = getHighestSectionPosition() + 15;
    bool foundLight = false;
    bool belowLimit = false;
    PIXBeginNamedEvent(0.0f, "Checking full height");
    for (;;)
    {
        int seaLevel = level->getSeaLevel();
        if (y <= seaLevel && (y < 1 || belowLimit))
        {
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Checking light emissions");
            for (int checkY = pos.y; checkY > 0; --checkY)
            {
                pos.y = checkY;
                Block* block = getBlock(pos);
                const BlockState* state = block->defaultBlockState();
                if (state->getBlock()->getLightEmission(state) > 0)
                    level->checkLight(pos, false, false);
            }
            PIXEndNamedEvent();
            PIXEndNamedEvent();
            return true;
        }
        PIXBeginNamedEvent(0.0f, "Checking at y %d", y);
        pos.y = y;
        int lightBlock = getBlockLightBlock(pos);
        if (lightBlock == 0xff && y < 0x3f)
            belowLimit = true;
        if (foundLight)
        {
            if (lightBlock == 0)
            {
                PIXBeginNamedEvent(0.0f, "First check light");
                if (!level->checkLight(pos, false, false))
                {
                    PIXEndNamedEvent();
                    PIXEndNamedEvent();
                    PIXEndNamedEvent();
                    PIXEndNamedEvent();
                    return false;
                }
                PIXEndNamedEvent();
            }
            PIXEndNamedEvent();
        }
        else
        {
            if (lightBlock >= 1)
                foundLight = true;
            PIXEndNamedEvent();
        }
        --y;
    }
}
Level* LevelChunk::getLevel()
{
    return level;
}
arrayWithLength<unsigned char> LevelChunk::getHeightmap()
{
    return mHeightmap;
}
void LevelChunk::recalcMinAndMaxHeights()
{
    mMinHeight = 255;
    mMaxHeight = 0;
    for (int i = 0; i < 256; ++i)
    {
        if (mHeightmap.data[i] < mMinHeight)
            mMinHeight = mHeightmap.data[i];
        if (mHeightmap.data[i] > mMaxHeight)
            mMaxHeight = mHeightmap.data[i];
    }
}
LevelChunk::BlockEntityMap* LevelChunk::getBlockEntities()
{
    return &mBlockEntityMap;
}
void LevelChunk::setLastSaveHadEntities(bool value)
{
    mLastSaveHadEntities = value;
}
void LevelChunk::setLastSaveTime(long long value)
{
    mLastSaveTime = value;
}
void LevelChunk::setInhabitedTime(long long value)
{
    mInhabitedTime = value;
}
void LevelChunk::compressLighting()
{
    if (mSkyLight != NULL)
        mSkyLight->compress();
    if (mSkyLightUpper != NULL)
        mSkyLightUpper->compress();
    if (mBlockLight != NULL)
        mBlockLight->compress();
    if (mBlockLightUpper != NULL)
        mBlockLightUpper->compress();
}
void LevelChunk::compressBlocks()
{
    if (mElements != NULL)
        mElements->removeCompressionHeadroom();
}
void LevelChunk::compressData() {}
void LevelChunk::setChunkVersion(int version)
{
    mChunkVersion = version;
}
void LevelChunk::swapEntities(LevelChunk* chunk)
{
    if (chunk != NULL)
    {
        std::vector<boost::shared_ptr<Entity>>** sections = mEntitySections;
        mEntitySections = chunk->mEntitySections;
        chunk->mEntitySections = sections;
    }
}
int LevelChunk::PackBlockPos(const BlockPos& pos)
{
    return (short)((pos.y & 255) | ((pos.z << 8) & 0xf00) | ((pos.x << 12) & 0xf000));
}
BlockPos LevelChunk::UnpackBlockPos(short packed)
{
    unsigned short value = (unsigned short)packed;
    return BlockPos((value >> 12) & 15, value & 255, (value >> 8) & 15);
}
void LevelChunk::CreateNewThreadStorage()
{
    ThreadStorage* storage = new ThreadStorage();
    storage->elementStorage = new unsigned short[0x10000];
    storage->extraElementStorage = new unsigned short[0x10000];
    storage->tempLightingStorage = new unsigned char[0x4080];
    TlsSetValue(sLevelChunkThreadStorage, storage);
}
void LevelChunk::ReleaseThreadStorage()
{
    ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage));
    delete[] storage->elementStorage;
    delete[] storage->extraElementStorage;
    delete[] storage->tempLightingStorage;
    delete storage;
}
