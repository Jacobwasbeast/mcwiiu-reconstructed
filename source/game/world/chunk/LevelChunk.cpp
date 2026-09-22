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
#include "game/world/block/CommandBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/world/block/Material.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/core/TickNextTickData.hpp"
#include "game/util/AABB.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/world/DimensionType.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/core/CMinecraftApp.hpp"
#include <cstring>
#include <cstdlib>

unsigned int TlsAlloc();
int TlsSetValue(unsigned int, void*);
void* TlsGetValue(unsigned int);

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

extern MinecraftServer* gMinecraftServer;
extern CMinecraftApp* g_pApp;

static unsigned int sLevelChunkThreadStorage = TlsAlloc();
static OSFastMutex sLevelChunkLightingMutex;
static OSFastMutex sLevelChunkEntityMutex;
static OSFastMutex sLevelChunkBlockEntityMutex;

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
    mServerChunkFlags = &source->mChunkFlags;
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
    int columnIndex = ((pos.z & 0xf) << 4) | (pos.x & 0xf);
    int cached = mPrecipitationHeightmap[columnIndex];
    if (cached != 255)
        return BlockPos(pos.x, cached, pos.z);
    int top = getHighestSectionPosition();
    BlockPos current(pos.x, top + 15, pos.z);
    int found = -1;
    while (current.y >= 0 && found == -1)
    {
        if (current.y < 256)
        {
            const BlockState* state = getBlockState(current);
            const BlockState* extraState = getExtraBlockState(current);
            Material* material = state->getMaterial();
            Material* extraMaterial = extraState->getMaterial();
            bool passable = !material->blocksMotion() && !material->isLiquid() &&
                            !extraMaterial->blocksMotion() && !extraMaterial->isLiquid();
            if (passable)
                current = current.below();
            else
                found = current.y + 1;
        }
        else
        {
            current = current.below();
        }
    }
    if (found == 255)
        found = 254;
    mPrecipitationHeightmap[columnIndex] = (unsigned char)found;
    return BlockPos(pos.x, found & 0xff, pos.z);
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
    unsigned char* buffer = (unsigned char*)malloc(getRawDataMaxBytes());
    unsigned char* cursor = buffer + 12;
    unsigned int elementBytes = mElements->getRawData(cursor);
    *(unsigned int*)buffer = elementBytes;
    cursor += elementBytes;
    if (mBlockLight != NULL)
    {
        unsigned short bytes = mBlockLight->getRawData(cursor);
        cursor += bytes;
        *(unsigned short*)(buffer + 4) = bytes;
    }
    if (mBlockLightUpper != NULL)
    {
        unsigned short bytes = mBlockLightUpper->getRawData(cursor);
        cursor += bytes;
        *(unsigned short*)(buffer + 6) = bytes;
    }
    if (mSkyLight != NULL)
    {
        unsigned short bytes = mSkyLight->getRawData(cursor);
        cursor += bytes;
        *(unsigned short*)(buffer + 8) = bytes;
    }
    if (mSkyLightUpper != NULL)
    {
        unsigned short bytes = mSkyLightUpper->getRawData(cursor);
        cursor += bytes;
        *(unsigned short*)(buffer + 10) = bytes;
    }
    memcpy(cursor, mBiomes.data, 256);
    cursor += 256;
    unsigned int totalBytes = (unsigned int)(cursor - buffer);
    arrayWithLength<unsigned char> result(totalBytes, false);
    memcpy(result.data, buffer, totalBytes);
    free(buffer);
    return result;
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
    if (mSkyLightUpper != NULL)
        mSkyLightUpper->write(stream);
}
void LevelChunk::writeCompressedBlockLightData(DataOutputStream* stream)
{
    if (mBlockLight != NULL)
        mBlockLight->write(stream);
    if (mBlockLightUpper != NULL)
        mBlockLightUpper->write(stream);
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
    if (mSkyLightUpper == NULL)
        mSkyLightUpper = new SparseLightStorage(true, true);
    unsigned char* buffer = GetTempLightingStorage();
    mSkyLight->read(stream, buffer);
    mSkyLightUpper->read(stream, buffer);
}
void LevelChunk::readCompressedBlockLightData(DataInputStream* stream)
{
    if (mBlockLight == NULL)
        mBlockLight = new SparseLightStorage(false);
    if (mBlockLightUpper == NULL)
        mBlockLightUpper = new SparseLightStorage(false, true);
    unsigned char* buffer = GetTempLightingStorage();
    mBlockLight->read(stream, buffer);
    mBlockLightUpper->read(stream, buffer);
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
    if (level->mDimension->getType() == DimensionType::NETHER)
    {
        level->getChunk(x - 1, z - 1)->reSyncLighting();
        level->getChunk(x, z - 1)->reSyncLighting();
        level->getChunk(x + 1, z - 1)->reSyncLighting();
        level->getChunk(x - 1, z)->reSyncLighting();
        level->getChunk(x + 1, z)->reSyncLighting();
        level->getChunk(x - 1, z + 1)->reSyncLighting();
        level->getChunk(x, z + 1)->reSyncLighting();
        level->getChunk(x + 1, z + 1)->reSyncLighting();
    }
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
        EnterCriticalSection(&sLevelChunkBlockEntityMutex);
        for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
            level->markForRemoval(it->second);
        mBlockEntityMap.clear();
        LeaveCriticalSection(&sLevelChunkBlockEntityMutex);
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
    mServerChunkFlags = NULL;
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
bool LevelChunk::isLoaded()
{
    return mLoaded;
}
void LevelChunk::setLoaded(bool loaded)
{
    mLoaded = loaded;
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
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmapOnly - Begin");
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmapOnly - getHighestNonEmptyY");
    int highestY = getHighestNonEmptyY();
    PIXEndNamedEvent();
    int rangeEnd = highestY / 4;
    if (highestY & 3)
        ++rangeEnd;
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmapOnly - getElements");
    unsigned short* elements = GetElementStorage();
    unsigned short* extraElements = GetExtraElementStorage();
    bool ownsBuffers = (elements == NULL);
    if (ownsBuffers)
    {
        elements = new unsigned short[0x10000];
        extraElements = new unsigned short[0x10000];
    }
    memset(elements, 0, 0x20000);
    memset(extraElements, 0, 0x20000);
    getElementRange(arrayWithLength<unsigned short>(elements, 0x10000),
                     arrayWithLength<unsigned short>(extraElements, 0x10000), 0, rangeEnd);
    PIXEndNamedEvent();
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmapOnly - rangeLoop");
    int runningMin = 255;
    int runningMax = 0;
    for (int localX = 0; localX < 16; ++localX)
    {
        for (int localZ = 0; localZ < 16; ++localZ)
        {
            mPrecipitationHeightmap[(localZ << 4) | localX] = 255;
            int height = highestY;
            if (highestY > 0)
            {
                int lightBlock;
                do
                {
                    int blockY = height - 1;
                    int index = ((blockY & 0x80) << 8) | (localX << 11) | (localZ << 7) | (blockY & 0x7f);
                    Block* block = Block::byId(elements[index] >> 4);
                    Block* extraBlock = Block::byId(extraElements[index] >> 4);
                    lightBlock = block->defaultBlockState()->getLightBlock();
                    int extraLightBlock = extraBlock->defaultBlockState()->getLightBlock();
                    if (extraLightBlock > lightBlock)
                        lightBlock = extraLightBlock;
                } while (lightBlock == 0 && --height > 0);
            }
            mHeightmap.data[(localZ << 4) | localX] = (unsigned char)height;
            if (height < runningMin)
                runningMin = height;
            if (runningMax < height)
                runningMax = height;
        }
    }
    PIXEndNamedEvent();
    mMaxHeight = runningMax;
    mMinHeight = runningMin;
    setUnsaved(true);
    if (ownsBuffers)
    {
        delete[] elements;
        delete[] extraElements;
    }
    PIXEndNamedEvent();
}
void LevelChunk::lightGaps(int x, int z)
{
    int index = (x >> 1) | (z << 3);
    mBiomeFlags[index] |= (unsigned char)(1 << ((x & 1) << 2));
    mHasGapsToCheck = true;
}
void LevelChunk::recalcHeightmap()
{
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - Begin");
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - getHighestNonEmptyY");
    int highestY = getHighestNonEmptyY();
    PIXEndNamedEvent();
    int rangeEnd = highestY / 4;
    if (highestY & 3)
        ++rangeEnd;
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - getElementRange");
    unsigned short* elements = GetElementStorage();
    unsigned short* extraElements = GetExtraElementStorage();
    bool ownsBuffers = (elements == NULL);
    if (ownsBuffers)
    {
        elements = new unsigned short[0x10000];
        extraElements = new unsigned short[0x10000];
    }
    memset(elements, 0, 0x20000);
    memset(extraElements, 0, 0x20000);
    getElementRange(arrayWithLength<unsigned short>(elements, 0x10000),
                     arrayWithLength<unsigned short>(extraElements, 0x10000), 0, rangeEnd);
    PIXEndNamedEvent();
    int runningMin = 255;
    int runningMax = 0;
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - RangedLoop");
    for (int localX = 0; localX < 16; ++localX)
    {
        for (int localZ = 0; localZ < 16; ++localZ)
        {
            PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - RangedLoop - findY");
            int height = highestY;
            if (highestY > 0)
            {
                int lightBlock;
                do
                {
                    --height;
                    int index = ((height & 0x80) << 8) | (localX << 11) | (localZ << 7) | (height & 0x7f);
                    Block* block = Block::byId(elements[index] >> 4);
                    Block* extraBlock = Block::byId(extraElements[index] >> 4);
                    lightBlock = block->defaultBlockState()->getLightBlock();
                    int extraLightBlock = extraBlock->defaultBlockState()->getLightBlock();
                    if (extraLightBlock > lightBlock)
                        lightBlock = extraLightBlock;
                } while (lightBlock == 0 && height > 0);
            }
            mHeightmap.data[(localZ << 4) | localX] = (unsigned char)height;
            if (height < runningMin)
                runningMin = height;
            if (runningMax < height)
                runningMax = height;
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - RangedLoop - setSkyLight");
            if (level->mDimension->isHasSkyLight())
            {
                SparseLightStorage* storage = mSkyLightUpper;
                int y = 255;
                int light = 15;
                for (;;)
                {
                    int lightBlock;
                    if (highestY < y)
                    {
                        lightBlock = (light != 15) ? 1 : 0;
                    }
                    else
                    {
                        int index = ((y & 0x80) << 8) | (localX << 11) | (localZ << 7) | (y & 0x7f);
                        Block* block = Block::byId(elements[index] >> 4);
                        Block* extraBlock = Block::byId(extraElements[index] >> 4);
                        lightBlock = block->defaultBlockState()->getLightBlock();
                        int extraLightBlock = extraBlock->defaultBlockState()->getLightBlock();
                        if (extraLightBlock > lightBlock)
                            lightBlock = extraLightBlock;
                        if (lightBlock == 0)
                            lightBlock = (light != 15) ? 1 : 0;
                    }
                    light -= lightBlock;
                    if (light > 0)
                        storage->set(localX, y & 0x7f, localZ, light);
                    --y;
                    if (y < 128)
                    {
                        storage = mSkyLight;
                        if (y < 1 || light < 1)
                            break;
                    }
                    else
                    {
                        storage = mSkyLightUpper;
                        if (light < 1)
                            break;
                    }
                }
            }
            PIXEndNamedEvent();
        }
    }
    PIXEndNamedEvent();
    mMaxHeight = runningMax;
    mMinHeight = runningMin;
    PIXBeginNamedEvent(0.0f, "LevelChunk::recalcHeightmap - lightGaps");
    for (int lx = 0; lx < 16; ++lx)
        for (int lz = 0; lz < 16; ++lz)
            lightGaps(lx, lz);
    PIXEndNamedEvent();
    setUnsaved(true);
    if (ownsBuffers)
    {
        delete[] elements;
        delete[] extraElements;
    }
    PIXEndNamedEvent();
}
Block* LevelChunk::getChunkBlock(int x, int y, int z)
{
    return Block::byId(getBlockId(x, y, z));
}
void LevelChunk::lightLava()
{
    if (mEmissiveMaxHeight == 0)
        return;
    for (int localX = 0; localX < 16; ++localX)
    {
        for (int zPair = 0; zPair < 8; ++zPair)
        {
            int localZ = zPair * 2;
            for (int localY = 0; localY < (int)mEmissiveMaxHeight; ++localY)
            {
                Block* block = getChunkBlock(localX, localY, localZ);
                int lightEmission = block->defaultBlockState()->getLightEmission();
                if (0 < lightEmission)
                {
                    int lightBlock = getBrightness(LightLayer::BLOCK, BlockPos(localX, localY, localZ));
                    if (lightBlock < lightEmission)
                        level->checkLight(LightLayer::BLOCK, BlockPos(x * 16 + localX, localY, z * 16 + localZ), true,
                                          false);
                }
            }
            int localZ2 = localZ + 1;
            for (int localY = 0; localY < (int)mEmissiveMaxHeight; ++localY)
            {
                Block* block = getChunkBlock(localX, localY, localZ2);
                int lightEmission = block->defaultBlockState()->getLightEmission();
                if (0 < lightEmission)
                {
                    int lightBlock = getBrightness(LightLayer::BLOCK, BlockPos(localX, localY, localZ2));
                    if (lightBlock < lightEmission)
                        level->checkLight(LightLayer::BLOCK, BlockPos(x * 16 + localX, localY, z * 16 + localZ2),
                                          true, false);
                }
            }
        }
    }
    if (level->mDimension->getType() == DimensionType::NETHER)
    {
        int worldX = x * 16;
        int worldZ = z * 16;
        bool east = level->hasChunk(x + 1, z, true);
        bool west = level->hasChunk(x - 1, z, true);
        bool south = level->hasChunk(x, z + 1, true);
        bool north = level->hasChunk(x, z - 1, true);
        if (west || east || north || south)
        {
            for (int worldY = 0; worldY < 256; ++worldY)
            {
                if (west || east)
                {
                    for (int offset = 0; offset < 16; ++offset)
                    {
                        if (west)
                            level->checkLight(LightLayer::BLOCK, BlockPos(worldX, worldY, worldZ + offset), true,
                                              false);
                        if (east)
                            level->checkLight(LightLayer::BLOCK, BlockPos(worldX + 15, worldY, worldZ + offset),
                                              true, false);
                    }
                }
                if (north || south)
                {
                    for (int offset = 0; offset < 16; ++offset)
                    {
                        if (north)
                            level->checkLight(LightLayer::BLOCK, BlockPos(worldX + offset, worldY, worldZ), true,
                                              false);
                        if (south)
                            level->checkLight(LightLayer::BLOCK, BlockPos(worldX + offset, worldY, worldZ + 15),
                                              true, false);
                    }
                }
            }
        }
    }
    mEmissiveMaxHeight = 0;
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
void LevelChunk::lightGap(int x, int z, int targetHeight)
{
    int height = level->getHeightmapPos(BlockPos(x, 0, z)).y;
    if (height < targetHeight)
        lightGap(x, z, height, targetHeight + 1);
    else if (targetHeight < height)
        lightGap(x, z, targetHeight, height + 1);
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
    while (height > 0 && getChunkBlockLightBlock(localX, height - 1, localZ) == 0)
        --height;
    if (height == oldHeight)
        return;
    mHeightmap.data[(localZ << 4) | localX] = (unsigned char)height;
    recalcMinAndMaxHeights();
    int worldX = x * 16 + localX;
    int worldZ = z * 16 + localZ;
    int lightColumnLow = height;
    if (level->mDimension->isHasSkyLight())
    {
        int lightY;
        if (height < oldHeight)
        {
            for (lightY = height; lightY < oldHeight; ++lightY)
            {
                if (lightY < 128)
                    mSkyLight->set(localX, lightY & 127, localZ, 15);
                else
                    mSkyLightUpper->set(localX, lightY & 127, localZ, 15);
            }
        }
        else if (oldHeight < height)
        {
            for (lightY = oldHeight; lightY < height; ++lightY)
            {
                if (lightY < 128)
                    mSkyLight->set(localX, lightY & 127, localZ, 0);
                else
                    mSkyLightUpper->set(localX, lightY & 127, localZ, 0);
            }
        }
        int remaining = 15;
        int endY = height;
        while (endY > 0 && remaining > 0)
        {
            --endY;
            int lightBlock = getChunkBlockLightBlock(localX, endY, localZ);
            if (lightBlock == 0)
                lightBlock = 1;
            remaining -= lightBlock;
            if (remaining < 0)
                remaining = 0;
            if (endY < 128)
                mSkyLight->set(localX, endY & 127, localZ, remaining);
            else
                mSkyLightUpper->set(localX, endY & 127, localZ, remaining);
        }
        lightColumnLow = endY;
    }
    level->lightColumnChanged(worldX, worldZ, lightColumnLow, oldHeight);
    int lowY = height < oldHeight ? height : oldHeight;
    int highY = height < oldHeight ? oldHeight : height;
    if (level->mDimension->isHasSkyLight())
    {
        PIXBeginNamedEvent(0.0f, "Light gaps");
        lightGap(worldX - 1, worldZ, lowY, highY);
        lightGap(worldX + 1, worldZ, lowY, highY);
        lightGap(worldX, worldZ - 1, lowY, highY);
        lightGap(worldX, worldZ + 1, lowY, highY);
        lightGap(worldX, worldZ, lowY, highY);
        PIXEndNamedEvent();
    }
    setUnsaved(true);
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
    int localX = pos.x & 15;
    int localZ = pos.z & 15;
    int columnIndex = (localZ << 4) | localX;
    if ((int)(mPrecipitationHeightmap[columnIndex] - 1) <= pos.y)
        mPrecipitationHeightmap[columnIndex] = 255;
    int existingHeight = mHeightmap.data[columnIndex];
    unsigned short oldValue = extra ? mElements->getExtraElement(localX, pos.y, localZ)
                                     : mElements->get(localX, pos.y, localZ, NULL);
    const BlockState* oldState = Block::getStateByElementId(oldValue);
    if (oldState == state)
    {
        boost::shared_ptr<BlockEntity> pending = getBlockEntity(pos, NO_CREATE);
        return false;
    }
    Block* newBlock = state->getBlock();
    int newBlockId = Block::getId(newBlock);
    unsigned char legacyData = (unsigned char)newBlock->convertBlockStateToLegacyData(state);
    Block* oldBlock = oldState->getBlock();
    int oldLightBlock = getBlockState(pos)->getLightBlock();
    int oldExtraLightBlock = getExtraBlockState(pos)->getLightBlock();
    if (oldLightBlock < oldExtraLightBlock)
        oldLightBlock = oldExtraLightBlock;
    bool commandBlockChange = false;
    int newLightBlock = 0;
    if (!extra)
    {
        mElements->set(localX, pos.y, localZ, (unsigned short)((newBlockId << 4 | legacyData) & 0xffff));
        if (oldBlock != newBlock)
        {
            int newLightEmission = newBlock->defaultBlockState()->getLightEmission();
            if (0 < newLightEmission && mEmissiveMaxHeight < pos.y + 1)
                mEmissiveMaxHeight = (unsigned short)(pos.y + 1);
            if (level->mIsClientSide)
            {
                if (oldBlock->mBlockEntity)
                {
                    if (CommandBlock::isCommandBlock(oldBlock) && CommandBlock::isCommandBlock(newBlock))
                        commandBlockChange = true;
                    else
                        level->removeBlockEntity(pos);
                }
            }
            else
            {
                oldBlock->onRemove(level, pos, state);
            }
        }
        PIXBeginNamedEvent(0.0f, "Updating lighting");
        newLightBlock = getBlockState(pos)->getLightBlock();
        int extraLightBlock = getExtraBlockState(pos)->getLightBlock();
        if (newLightBlock < extraLightBlock)
            newLightBlock = extraLightBlock;
        if (!newBlock->canContainLiquid(state))
        {
            mElements->setExtraElement(localX, pos.y, localZ, 0);
        }
        if (0 < newLightBlock)
        {
            if (existingHeight <= pos.y)
            {
                PIXBeginNamedEvent(0.0f, "Recalc height 1");
                recalcHeight(localX, pos.y + 1, localZ);
                PIXEndNamedEvent();
            }
        }
        else if (pos.y == existingHeight - 1)
        {
            PIXBeginNamedEvent(0.0f, "Recalc height 2");
            recalcHeight(localX, pos.y, localZ);
            PIXEndNamedEvent();
        }
    }
    else
    {
        if (newBlockId == 0)
            mElements->setExtraElement(localX, pos.y, localZ, (unsigned short)(legacyData & 0xffff));
        PIXBeginNamedEvent(0.0f, "Updating lighting");
        newLightBlock = getBlockState(pos)->getLightBlock();
        int extraLightBlock = getExtraBlockState(pos)->getLightBlock();
        if (newLightBlock < extraLightBlock)
            newLightBlock = extraLightBlock;
        if (0 < newLightBlock)
        {
            if (existingHeight <= pos.y)
            {
                PIXBeginNamedEvent(0.0f, "Recalc height 1");
                recalcHeight(localX, pos.y + 1, localZ);
                PIXEndNamedEvent();
            }
        }
        else if (pos.y == existingHeight - 1)
        {
            PIXBeginNamedEvent(0.0f, "Recalc height 2");
            recalcHeight(localX, pos.y, localZ);
            PIXEndNamedEvent();
        }
    }
    if (newLightBlock < oldLightBlock ||
        (oldLightBlock < newLightBlock &&
         (getBrightness(LightLayer::SKY, pos) > 0 || getBrightness(LightLayer::BLOCK, pos) > 0)))
    {
        PIXBeginNamedEvent(0.0f, "Lighting gaps");
        lightGaps(localX, localZ);
        PIXEndNamedEvent();
    }
    if (oldBlock->mBlockEntity && !commandBlockChange)
    {
        boost::shared_ptr<BlockEntity> pending = getBlockEntity(pos, NO_CREATE);
    }
    PIXEndNamedEvent();
    if (!level->mIsClientSide && oldBlock != newBlock)
    {
        newBlock->onPlace(level, pos, state);
    }
    else if (newBlock == Blocks::FIRE && !newBlock->mayPlace(level, pos))
    {
        mElements->set(localX, pos.y & 0xff, localZ, 0);
    }
    if (newBlock->mBlockEntity)
    {
        boost::shared_ptr<BlockEntity> pending = getBlockEntity(pos, NO_CREATE);
        if (!pending)
        {
            boost::shared_ptr<BlockEntity> created = createBlockEntity(pos);
            level->setBlockEntity(pos, created);
        }
    }
    setUnsaved(true);
    return true;
}
bool LevelChunk::setBlockAndData(int x, int y, int z, int blockId, int data, bool update, bool extra)
{
    int localX = x & 15;
    int localZ = z & 15;
    BlockPos pos(this->x * 16 + x, y, this->z * 16 + z);
    int columnIndex = (localZ << 4) | localX;
    if ((int)(mPrecipitationHeightmap[columnIndex] - 1) <= y)
        mPrecipitationHeightmap[columnIndex] = 255;
    if (extra)
    {
        if (mElements != NULL)
            mElements->set(localX, y, localZ, (unsigned short)((blockId << 4 | data) & 0xffff));
        return true;
    }
    int existingHeight = mHeightmap.data[columnIndex];
    unsigned short oldValue = mElements->get(localX, y & 0xff, localZ, NULL);
    int oldBlockId = oldValue >> 4;
    const BlockState* oldState = Block::getStateByElementId(oldValue);
    if (oldBlockId == blockId && (oldValue & 0xf) == data)
    {
        boost::shared_ptr<BlockEntity> pending = getBlockEntity(pos, NO_CREATE);
        return false;
    }
    Block* oldBlock = Block::byId(oldBlockId);
    int oldLightBlock = getBlockState(pos)->getLightBlock();
    int oldExtraLightBlock = getExtraBlockState(pos)->getLightBlock();
    if (oldLightBlock < oldExtraLightBlock)
        oldLightBlock = oldExtraLightBlock;
    Block* newBlock = Block::byId(blockId);
    bool commandBlockChange = false;
    if (oldBlockId != 0)
    {
        if (level->mIsClientSide)
        {
            if (oldBlock->mBlockEntity && oldBlockId != blockId)
            {
                if (CommandBlock::isCommandBlock(oldBlock) && CommandBlock::isCommandBlock(newBlock))
                    commandBlockChange = true;
                else
                    level->removeBlockEntity(pos);
            }
        }
        else
        {
            oldBlock->onRemove(level, pos, oldState);
        }
    }
    if (mElements != NULL)
        mElements->set(localX, y, localZ, (unsigned short)((blockId << 4 | data) & 0xffff));
    const BlockState* newState = newBlock->getBlockState(data);
    if (!newBlock->canContainLiquid(newState))
        mElements->setExtraElement(localX, y, localZ, 0);
    int newLightEmission = newBlock->defaultBlockState()->getLightEmission();
    if (0 < newLightEmission && mEmissiveMaxHeight < y + 1)
        mEmissiveMaxHeight = (unsigned short)(y + 1);
    PIXBeginNamedEvent(0.0f, "Updating lighting");
    int newLightBlock = getBlockState(pos)->getLightBlock();
    int newExtraLightBlock = getExtraBlockState(pos)->getLightBlock();
    if (newLightBlock < newExtraLightBlock)
        newLightBlock = newExtraLightBlock;
    if (update && newLightBlock != oldLightBlock)
    {
        if (level->mDimension->isHasSkyLight())
        {
            if (newLightBlock == 0)
            {
                if (y == existingHeight - 1)
                {
                    PIXBeginNamedEvent(0.0f, "Recalc height 2");
                    recalcHeight(localX, y, localZ);
                    PIXEndNamedEvent();
                }
            }
            else if (existingHeight <= y)
            {
                PIXBeginNamedEvent(0.0f, "Recalc height 1");
                recalcHeight(localX, y + 1, localZ);
                PIXEndNamedEvent();
            }
        }
        PIXBeginNamedEvent(0.0f, "Lighting gaps");
        lightGaps(localX, localZ);
        PIXEndNamedEvent();
    }
    PIXEndNamedEvent();
    if (blockId >= 0)
    {
        if (blockId < 1)
        {
            if (0 < oldBlockId && oldBlock->mBlockEntity)
            {
                boost::shared_ptr<BlockEntity> pending = getBlockEntity(pos, NO_CREATE);
            }
        }
        else if (newBlock->mBlockEntity && !commandBlockChange)
        {
            boost::shared_ptr<BlockEntity> pending = getBlockEntity(pos, NO_CREATE);
            if (!pending)
            {
                EntityBlock* entityBlock = dynamic_cast<EntityBlock*>(newBlock);
                boost::shared_ptr<BlockEntity> created = entityBlock->newBlockEntity(level, data);
                level->setBlockEntity(pos, created);
            }
        }
    }
    setUnsaved(true);
    return true;
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
    brightnesses[4] = getBrightness(layer, BlockPos(x, y, z - 1));
    brightnesses[5] = getBrightness(layer, BlockPos(x, y, z + 1));
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
    boost::shared_ptr<BlockEntity> result;
    EnterCriticalSection(&sLevelChunkBlockEntityMutex);
    BlockEntityMap::iterator it = mBlockEntityMap.find(pos);
    if (it == mBlockEntityMap.end())
    {
        LeaveCriticalSection(&sLevelChunkBlockEntityMutex);
        if (level->mScheduledUpdatesAreImmediate)
            return boost::shared_ptr<BlockEntity>();
        if (type == IMMEDIATE)
        {
            result = createBlockEntity(pos);
            level->setBlockEntity(pos, result);
        }
        else if (type == QUEUED)
        {
            mPendingBlockEntities.push_back(pos);
        }
        EnterCriticalSection(&sLevelChunkBlockEntityMutex);
        it = mBlockEntityMap.find(pos);
    }
    if (it != mBlockEntityMap.end())
        result = it->second;
    LeaveCriticalSection(&sLevelChunkBlockEntityMutex);
    if (result && result->isRemoved())
    {
        EnterCriticalSection(&sLevelChunkBlockEntityMutex);
        mBlockEntityMap.erase(pos);
        LeaveCriticalSection(&sLevelChunkBlockEntityMutex);
        return boost::shared_ptr<BlockEntity>();
    }
    return result;
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
    entity->setLevel(level);
    entity->setPosition(pos);
    Block* block = getBlock(pos);
    if (block->mBlockEntity)
    {
        BlockEntityMap::iterator old = mBlockEntityMap.find(pos);
        if (old != mBlockEntityMap.end())
            old->second->setRemoved();
        entity->clearRemoved();
        EnterCriticalSection(&sLevelChunkBlockEntityMutex);
        mBlockEntityMap[pos] = entity;
        LeaveCriticalSection(&sLevelChunkBlockEntityMutex);
    }
}
void LevelChunk::removeBlockEntity(const BlockPos& pos)
{
    EnterCriticalSection(&sLevelChunkBlockEntityMutex);
    BlockEntityMap::iterator it = mBlockEntityMap.find(pos);
    if (it != mBlockEntityMap.end())
    {
        boost::shared_ptr<BlockEntity> removed = it->second;
        mBlockEntityMap.erase(pos);
        if (removed.get() != NULL)
            removed->setRemoved();
    }
    LeaveCriticalSection(&sLevelChunkBlockEntityMutex);
}
void LevelChunk::load(bool blockEntitiesOnly)
{
    mLoaded = true;
    SetInvalid(false);
    if (level->mIsClientSide && !blockEntitiesOnly)
    {
        mUnloaded = false;
        return;
    }
    if (mUnloaded && mPendingBlockEntityTag != NULL)
    {
        ListTag* entities = mPendingBlockEntityTag->getList(L"Entities", 10);
        if (entities != NULL)
        {
            for (int i = 0; i < entities->size(); ++i)
            {
                CompoundTag* tag = entities->getCompound(i);
                boost::shared_ptr<Entity> entity = EntityIO::newEntity(tag, level);
                if (entity)
                {
                    entity->onLoadedFromSave();
                    addEntity(entity);
                    if (tag->contains(L"Riding", 9))
                    {
                        ListTag* riders = tag->getList(L"Riding", 10);
                        for (int j = 0; j < riders->size(); ++j)
                        {
                            boost::shared_ptr<Entity> rider = EntityIO::newEntity(riders->getCompound(j), level);
                            if (rider)
                            {
                                addEntity(rider);
                                rider->startRiding(entity, true);
                            }
                        }
                    }
                }
            }
        }
        ListTag* blockEntities = mPendingBlockEntityTag->getList(L"TileEntities", 10);
        if (blockEntities != NULL)
        {
            for (int i = 0; i < blockEntities->size(); ++i)
            {
                boost::shared_ptr<BlockEntity> entity = BlockEntity::loadStatic(level, blockEntities->getCompound(i));
                if (entity)
                    addBlockEntity(entity);
            }
        }
        ListTag* ticks = mPendingBlockEntityTag->getList(L"TileTicks", 10);
        if (ticks != NULL)
        {
            for (int i = 0; i < ticks->size(); ++i)
            {
                CompoundTag* tag = ticks->getCompound(i);
                Block* block = tag->contains(L"i", 8) ? Block::byString(tag->getString(L"i")) : Block::byId(tag->getInt(L"i"));
                const int tickX = tag->getInt(L"x");
                const int tickY = tag->getInt(L"y");
                const int tickZ = tag->getInt(L"z");
                const BlockPos pos(tickX, tickY, tickZ);
                const int delay = tag->getInt(L"t");
                const int priority = tag->getInt(L"p");
                level->forceAddBlockTick(pos, block, delay, priority);
            }
        }
        delete mPendingBlockEntityTag;
        mPendingBlockEntityTag = NULL;
    }
    mUnloaded = false;
    std::vector<boost::shared_ptr<BlockEntity> > blockEntities;
    EnterCriticalSection(&sLevelChunkBlockEntityMutex);
    for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
        blockEntities.push_back(it->second);
    LeaveCriticalSection(&sLevelChunkBlockEntityMutex);
    level->addAllPendingBlockEntities(blockEntities);
    if (!blockEntitiesOnly)
    {
        EnterCriticalSection(&sLevelChunkEntityMutex);
        for (int i = 0; i < mEntitySectionCount; ++i)
            level->addEntities(mEntitySections[i]);
        LeaveCriticalSection(&sLevelChunkEntityMutex);
    }
}
void LevelChunk::unload(bool removeBlockEntities, bool removeEntities)
{
    mLoaded = false;
    if (removeBlockEntities)
    {
        EnterCriticalSection(&sLevelChunkBlockEntityMutex);
        for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
            level->markForRemoval(it->second);
        LeaveCriticalSection(&sLevelChunkBlockEntityMutex);
    }
    if (removeEntities)
    {
        EnterCriticalSection(&sLevelChunkEntityMutex);
        for (int i = 0; i < mEntitySectionCount; ++i)
            level->removeEntities(mEntitySections[i]);
        LeaveCriticalSection(&sLevelChunkEntityMutex);
    }
    if (!mUnloaded)
    {
        mUnloaded = true;
        if (!level->mIsClientSide)
        {
            delete mPendingBlockEntityTag;
            mPendingBlockEntityTag = new CompoundTag();
            PIXBeginNamedEvent(0.0f, "Saving entities");
            ListTag* entities = new ListTag();
            EnterCriticalSection(&sLevelChunkEntityMutex);
            for (int i = 0; i < mEntitySectionCount; ++i)
            {
                std::vector<boost::shared_ptr<Entity> >& section = *mEntitySections[i];
                for (std::vector<boost::shared_ptr<Entity> >::iterator it = section.begin(); it != section.end(); ++it)
                {
                    boost::shared_ptr<Entity> entity = *it;
                    CompoundTag* tag = new CompoundTag();
                    if (entity->save(tag))
                        entities->add(tag);
                }
                section.clear();
            }
            LeaveCriticalSection(&sLevelChunkEntityMutex);
            mPendingBlockEntityTag->put(L"Entities", entities);
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Saving block entities");
            ListTag* blockEntities = new ListTag();
            for (BlockEntityMap::iterator it = mBlockEntityMap.begin(); it != mBlockEntityMap.end(); ++it)
            {
                boost::shared_ptr<BlockEntity> entity = it->second;
                CompoundTag* tag = new CompoundTag();
                entity->save(tag);
                blockEntities->add(tag);
            }
            mPendingBlockEntityTag->put(L"TileEntities", blockEntities);
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0f, "Saving block tick data");
            std::vector<TickNextTickData>* ticks = level->fetchTicksInChunk(this, false);
            if (ticks != NULL)
            {
                const long long gameTime = level->getGameTime();
                ListTag* savedTicks = new ListTag();
                for (std::size_t i = 0; i < ticks->size(); ++i)
                {
                    TickNextTickData tick = ticks->at(i);
                    CompoundTag* tag = new CompoundTag();
                    tag->putInt(L"i", Block::getId(tick.getBlock()));
                    tag->putInt(L"x", tick.pos.x);
                    tag->putInt(L"y", tick.pos.y);
                    tag->putInt(L"z", tick.pos.z);
                    tag->putInt(L"t", static_cast<int>(tick.triggerTick - gameTime));
                    tag->putInt(L"p", tick.priorityTilt);
                    savedTicks->add(tag);
                }
                mPendingBlockEntityTag->put(L"TileTicks", savedTicks);
                delete ticks;
            }
            PIXEndNamedEvent();
        }
        if (removeBlockEntities)
            mBlockEntityMap.clear();
    }
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
    EnterCriticalSection(&sLevelChunkEntityMutex);
    for (int section = 0; section < mEntitySectionCount; ++section)
        count += (int)mEntitySections[section]->size();
    LeaveCriticalSection(&sLevelChunkEntityMutex);
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
    InitializeCriticalSection(&sLevelChunkBlockEntityMutex);
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
    if (mChunkVersion >= 10)
        return;
    LevelChunk* chunk = source->getChunkLoadedOrUnloaded(chunkX, chunkZ);
    PIXBeginNamedEvent(0.0f, "LevelChunk::checkChests");
    BlockPos gatePos(0, 0, 0);
    for (int localX = 0; localX < 16; ++localX)
    {
        int worldX = chunkX * 16 + localX;
        for (int localZ = 0; localZ < 16; ++localZ)
        {
            int worldZ = chunkZ * 16 + localZ;
            for (int yPair = 0; yPair < 64; ++yPair)
            {
                int y0 = yPair * 2;
                const BlockState* state0 = chunk->getBlockState(localX, y0, localZ);
                if (state0->getBlock() == Blocks::chest && chunk->getData(gatePos) == 0)
                {
                    BlockPos pos(worldX, y0, worldZ);
                    PIXBeginNamedEvent(0.0f, "Recalc lock dir");
                    static_cast<ChestBlock*>(Blocks::chest)->recalcLockDir(level, pos, state0);
                    PIXEndNamedEvent();
                    PIXBeginNamedEvent(0.0f, "Check light");
                    level->checkLight(pos, true, false);
                    PIXEndNamedEvent();
                }
                int y1 = y0 + 1;
                const BlockState* state1 = chunk->getBlockState(localX, y1, localZ);
                if (state1->getBlock() == Blocks::chest && chunk->getData(gatePos) == 0)
                {
                    BlockPos pos(worldX, y1, worldZ);
                    PIXBeginNamedEvent(0.0f, "Recalc lock dir");
                    static_cast<ChestBlock*>(Blocks::chest)->recalcLockDir(level, pos, state1);
                    PIXEndNamedEvent();
                    PIXBeginNamedEvent(0.0f, "Check light");
                    level->checkLight(pos, true, false);
                    PIXEndNamedEvent();
                }
            }
        }
    }
    PIXEndNamedEvent();
}
void LevelChunk::tick(bool update)
{
    if (mHasGapsToCheck && level->mDimension->isHasSkyLight() && !update)
    {
        PIXBeginNamedEvent(0.0f, "Check for gaps");
        recheckGaps(level->mIsClientSide);
        PIXEndNamedEvent();
    }
    mLightPopulated = true;
    if (!mPostProcess && isTerrainPopulated())
    {
        PIXBeginNamedEvent(0.0f, "postProcess");
        postProcess();
        PIXEndNamedEvent();
    }
    PIXBeginNamedEvent(0.0f, "Process block entities");
    while (!mPendingBlockEntities.empty())
    {
        BlockPos pos = mPendingBlockEntities.front();
        mPendingBlockEntities.pop_front();
        bool shouldCreate = !getBlockEntity(pos, NO_CREATE)
            && getBlock(pos)->mBlockEntity;
        if (shouldCreate)
        {
            boost::shared_ptr<BlockEntity> entity = createBlockEntity(pos);
            level->setBlockEntity(pos, entity);
            level->setBlocksDirty(pos, pos);
        }
    }
    PIXEndNamedEvent();
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
    int localX = pos.x & 15;
    int localZ = pos.z & 15;
    int columnIndex = (localZ << 4) | localX;
    if (!level->mIsClientSide && mBiomes.data[columnIndex] == 255)
    {
        Biome* biome = source->getBiome(pos, Biome::PLAINS);
        int biomeId = Biome::getId(biome);
        mBiomes.data[columnIndex] = (unsigned char)biomeId;
        if (biomeId == 0)
            mBiomePresent |= 1;
        else if (biomeId == 6)
            mBiomePresent |= 8;
        else if (biomeId == 0x18)
            mBiomePresent |= 2;
    }
    Biome* result = Biome::getBiome(mBiomes.data[columnIndex]);
    if (result == NULL)
    {
        int dimensionId = level->mDimension->getType()->getId();
        if (dimensionId == 0)
            return Biome::PLAINS;
        if (dimensionId == 1)
            return Biome::THE_END;
        if (dimensionId == -1)
            return Biome::HELL;
    }
    return result;
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
