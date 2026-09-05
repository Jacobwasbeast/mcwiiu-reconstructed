#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/data/SparseDataStorage.hpp"

unsigned int TlsAlloc();
bool TlsSetValue(unsigned int, void*);
void* TlsGetValue(unsigned int);

static unsigned int sLevelChunkThreadStorage = TlsAlloc();
static OSFastMutex sLevelChunkLightingMutex;
static OSFastMutex sLevelChunkEntityMutex;
static OSFastMutex sLevelChunkPostProcessMutex;

LevelChunk::LevelChunk(Level* level, int x, int z) : level(level), x(x), z(z), mElements(new ElementStorage(false)), mSkyLight(NULL), mSkyLightUpper(new SparseLightStorage(true, true)), mBlockLight(NULL), mBlockLightUpper(new SparseLightStorage(false, true)), mUnsaved(false), mLoaded(false), mUnloaded(false), mTerrainPopulated(false), mInhabitedTime(0), mLastSaveTime(0), mLowestHeightmap(0), mBiomePresent(0), mPostProcess(0), mLightPopulated(0), mLastSaveHadEntities(0), mChunkVersion(0), mMinHeight(0), mMaxHeight(0) {}
LevelChunk::LevelChunk(Level* level, ChunkPrimer* primer, int x, int z) : level(level), x(x), z(z), mElements(primer->getElementIds().data == NULL ? new ElementStorage(level->mIsClientSide) : new ElementStorage(level->mIsClientSide, primer->getElementIds(), 4)), mSkyLight(primer->getElementIds().data == NULL ? NULL : new SparseLightStorage(true)), mSkyLightUpper(new SparseLightStorage(true, true)), mBlockLight(primer->getElementIds().data == NULL ? NULL : new SparseLightStorage(false)), mBlockLightUpper(new SparseLightStorage(false, true)), mUnsaved(false), mLoaded(false), mUnloaded(false), mTerrainPopulated(false), mInhabitedTime(0), mLastSaveTime(0), mLowestHeightmap(0), mBiomePresent(0), mPostProcess(0), mLightPopulated(0), mLastSaveHadEntities(0), mChunkVersion(0), mMinHeight(0), mMaxHeight(0) {}
LevelChunk::LevelChunk(Level* level, int x, int z, LevelChunk* source) : level(level), x(x), z(z), mElements(new ElementStorage(level->mIsClientSide, source->mElements)), mSkyLight(new SparseLightStorage(source->mSkyLight)), mSkyLightUpper(new SparseLightStorage(source->mSkyLightUpper)), mBlockLight(new SparseLightStorage(source->mBlockLight)), mBlockLightUpper(new SparseLightStorage(source->mBlockLightUpper)), mUnsaved(false), mLoaded(false), mUnloaded(false), mTerrainPopulated(false), mInhabitedTime(0), mLastSaveTime(0), mLowestHeightmap(0), mBiomePresent(0), mPostProcess(0), mLightPopulated(0), mLastSaveHadEntities(0), mChunkVersion(0), mMinHeight(0), mMaxHeight(0) {}
LevelChunk::~LevelChunk() { delete mElements; delete mSkyLight; delete mSkyLightUpper; delete mBlockLight; delete mBlockLightUpper; }
int LevelChunk::BlockChange::GetPackedPos() const { return mPackedPos; }
unsigned int LevelChunk::BlockChange::GetElement() const { return (unsigned int)mElement; }
BlockPos LevelChunk::BlockChange::GetPos() const { return LevelChunk::UnpackBlockPos((short)mPackedPos); }
const BlockState* LevelChunk::BlockChange::GetState() const { return Block::getStateByElementId((unsigned short)(mElement & 255)); }
const BlockState* LevelChunk::BlockChange::GetExtraState() const { return Block::getStateByElementId((unsigned short)((mElement >> 16) & 255)); }
LevelChunk::ThreadStorage::ThreadStorage() : elementStorage(NULL), extraElementStorage(NULL), tempLightingStorage(NULL) {}

bool LevelChunk::isReady() { return mPostProcess && mTerrainPopulated && mLightPopulated; }
int LevelChunk::getBlockId(int x, int y, int z) { return (int)(mElements->get(x, y, z, NULL) >> 4); }
unsigned int LevelChunk::getLowestHeightmap() { return mLowestHeightmap; }
unsigned short LevelChunk::getElement(int x, int y, int z) { return (unsigned short)mElements->get(x, y, z, NULL); }
bool LevelChunk::sectionContainsBlockId(int x, int y, int z, int blockId) { return mElements->sectionContainsElementData(x, y, z, (unsigned short)((blockId & 0xfff) << 4), 0x7ff0); }
int LevelChunk::getExtraBlockId(int x, int y, int z) { return (int)(mElements->getExtraElement(x, y, z) >> 4); }
void LevelChunk::getBlockAndExtraId(int x, int y, int z, int* blockId, int* extraBlockId) { unsigned short element = 0; unsigned short extra = 0; mElements->getElementAndExtraElement(x, y, z, &element, &extra); *blockId = element >> 4; *extraBlockId = extra >> 4; }
unsigned int LevelChunk::getData(int x, int y, int z) { return mElements->get(x, y, z, NULL) & 15; }
unsigned int LevelChunk::getExtraBlockData(int x, int y, int z) { return mElements->getExtraElement(x, y, z) & 15; }
unsigned short LevelChunk::getElement(int x, int y, int z, bool* present) { return (unsigned short)mElements->get(x, y, z, present); }
int LevelChunk::getHighestSectionPosition() { for (int section = 15; section >= 0; --section) { int highest = 0; if (!mElements->isRenderChunkEmpty(section * 16, &highest)) return section * 16 + 15; } return 0; }
BlockPos LevelChunk::getTopRainBlock(const BlockPos& pos) { return BlockPos(pos.x, getHeightmap(pos), pos.z); }
void LevelChunk::updateBiomeFlags(int x, int z) { unsigned int index = ((x >> 1) | (z << 3)) & 127; mBiomeFlags[index] |= (unsigned char)(2 << ((x & 1) << 2)); }
bool LevelChunk::biomeHasRain(int x, int z) { updateBiomeFlags(x, z); return (mBiomeFlags[((x >> 1) | (z << 3)) & 127] & (8 << ((x & 1) << 2))) != 0; }
bool LevelChunk::biomeHasSnow(int x, int z) { updateBiomeFlags(x, z); return (mBiomeFlags[((x >> 1) | (z << 3)) & 127] & (4 << ((x & 1) << 2))) != 0; }
long long LevelChunk::getInhabitedTime() { return mInhabitedTime; }
bool LevelChunk::isTerrainPopulated() { return mTerrainPopulated; }
bool LevelChunk::isRenderChunkEmpty(int section) { int highest = 0; return mElements->isRenderChunkEmpty(section, &highest); }
void LevelChunk::getBlockData(arrayWithLength<unsigned char> data) { arrayWithLength<unsigned char> extra; mElements->getData(data, extra, 0, 64); }
void LevelChunk::setAllElements(arrayWithLength<unsigned char> elements, arrayWithLength<unsigned char> data) { mElements->setData(elements, data); }
void LevelChunk::setAllElements(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> data) { mElements->setData(elements, data); }
void LevelChunk::getAllElements(arrayWithLength<unsigned char> elements, arrayWithLength<unsigned char> data) { mElements->getData(elements, data, 0, 64); }
void LevelChunk::getElementRange(arrayWithLength<unsigned short> elements, int start, int end) { mElements->getData(elements, start, end); }
void LevelChunk::getElementRange(arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> data, int start, int end) { if (mElements != NULL) mElements->getData(elements, data, start, end); }
int LevelChunk::getRawDataMaxBytes() { return ElementStorage::getRawDataMaxBytes() + (int)SparseLightStorage::getRawDataMaxBytes() * 4 + 0x10c; }
arrayWithLength<unsigned char> LevelChunk::getRawData() { arrayWithLength<unsigned char> data(getRawDataMaxBytes(), false); data.length = mElements->getRawData(data.data); return data; }
void LevelChunk::setFromRawData(arrayWithLength<unsigned char> data) { mElements->setFromRawData(data.data, data.length); }
void LevelChunk::getDataData(arrayWithLength<unsigned char> data) { mElements->getData(data, data, 0, 256); }
void LevelChunk::getSkyLightData(arrayWithLength<unsigned char> data) { if (mSkyLight != NULL) mSkyLight->getData(data, 0); }
void LevelChunk::getBlockLightData(arrayWithLength<unsigned char> data) { if (mBlockLight != NULL) mBlockLight->getData(data, 0); }
void LevelChunk::setSkyLightData(arrayWithLength<unsigned char> data) { if (mSkyLight == NULL) mSkyLight = new SparseLightStorage(false); mSkyLight->setData(data, 0); }
void LevelChunk::setBlockLightData(arrayWithLength<unsigned char> data) { if (mBlockLight == NULL) mBlockLight = new SparseLightStorage(false); mBlockLight->setData(data, 0); }
void LevelChunk::setSkyLightDataAllBright() { if (mSkyLight == NULL) mSkyLight = new SparseLightStorage(true); else mSkyLight->setAllBright(); }
bool LevelChunk::isLowerBlockLightStorageCompressed() { return mBlockLight != NULL && mBlockLight->isCompressed(); }
void LevelChunk::writeCompressedElements(DataOutputStream* stream) { mElements->write(stream); }
void LevelChunk::readCompressedElements(DataInputStream* stream) { mElements->read(stream); }
void LevelChunk::readCompressedElementsFromBlockAndData(DataInputStream* stream) { CompressedBlockStorage blocks; CompressedBlockStorage extraBlocks; SparseDataStorage data; SparseDataStorage extraData; blocks.read(stream); extraBlocks.read(stream); data.read(stream); extraData.read(stream); delete mElements; mElements = new ElementStorage(false, &blocks, &extraBlocks, &data, &extraData); }
void LevelChunk::writeCompressedSkyLightData(DataOutputStream* stream) { if (mSkyLight != NULL) mSkyLight->write(stream); }
void LevelChunk::writeCompressedBlockLightData(DataOutputStream* stream) { if (mBlockLight != NULL) mBlockLight->write(stream); }
unsigned char* LevelChunk::GetTempLightingStorage() { ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage)); return storage == NULL ? NULL : storage->tempLightingStorage; }
void LevelChunk::readCompressedSkyLightData(DataInputStream* stream) { if (mSkyLight == NULL) mSkyLight = new SparseLightStorage(true); mSkyLight->read(stream, GetTempLightingStorage()); }
void LevelChunk::readCompressedBlockLightData(DataInputStream* stream) { if (mBlockLight == NULL) mBlockLight = new SparseLightStorage(false); mBlockLight->read(stream, GetTempLightingStorage()); }
unsigned int LevelChunk::getSparseBlockAndDataMemoryAllocated() { return mElements == NULL ? 0 : mElements->getAllocatedBytes(); }
bool LevelChunk::getHasGapsToCheck() { return mBiomePresent != 0; }
bool LevelChunk::getBiomePresentFlag() { return mBiomePresent != 0; }
void LevelChunk::reSyncLighting() { mLightPopulated = 1; }
bool LevelChunk::reSyncLightingNearby() { reSyncLighting(); return true; }
void LevelChunk::dropLighting() { delete mSkyLight; delete mSkyLightUpper; delete mBlockLight; delete mBlockLightUpper; mSkyLight = NULL; mSkyLightUpper = NULL; mBlockLight = NULL; mBlockLightUpper = NULL; }
void LevelChunk::SetInvalid(bool invalid) { mUnloaded = invalid; }
bool LevelChunk::hasLightingBeenDropped() { return mSkyLight == NULL && mBlockLight == NULL; }
void LevelChunk::init(Level* level, int x, int z) { this->level = level; this->x = x; this->z = z; mUnloaded = false; }
bool LevelChunk::isAt(int x, int z) { return x == this->x && z == this->z; }
int LevelChunk::getHeightmap(const BlockPos& pos) { return getHeightmap(pos.x & 15, pos.z & 15); }
unsigned char LevelChunk::getHeightmap(int x, int z) { return mHeightmap[(z << 4) | x]; }
void LevelChunk::recalcBlockLights() {}
int LevelChunk::getHighestNonEmptyY() { return mElements->getHighestNonEmptyY(); }
unsigned short* LevelChunk::GetElementStorage() { ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage)); return storage == NULL ? NULL : storage->elementStorage; }
unsigned short* LevelChunk::GetExtraElementStorage() { ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage)); return storage == NULL ? NULL : storage->extraElementStorage; }
void LevelChunk::setUnsaved(bool unsaved) { mUnsaved = unsaved; }
void LevelChunk::recalcHeightmapOnly() { recalcMinAndMaxHeights(); }
void LevelChunk::lightGaps(int x, int z) {}
void LevelChunk::recalcHeightmap() { recalcHeightmapOnly(); }
Block* LevelChunk::getChunkBlock(int x, int y, int z) { return Block::byId(getBlockId(x, y, z)); }
void LevelChunk::lightLava() { mLightPopulated = 1; }
void LevelChunk::lightGap(int x, int y, int z, int value) {}
bool LevelChunk::lightGap(int x, int y, int z) { (void)x; (void)y; (void)z; return true; }
void LevelChunk::recheckGaps(bool onlyOne) {}
void LevelChunk::recalcHeight(int x, int y, int z) {}
int LevelChunk::getBlockLightBlock(const BlockPos& pos) { int block = getBlock(pos)->defaultBlockState()->getLightBlock(); int extra = getExtraBlock(pos)->defaultBlockState()->getLightBlock(); return block > extra ? block : extra; }
int LevelChunk::getChunkBlockLightBlock(int x, int y, int z) { return getBlockLightBlock(BlockPos(this->x * 16 + x, y, this->z * 16 + z)); }
Block* LevelChunk::getBlock(const BlockPos& pos) { return Block::byId(getBlockId(pos.x & 15, pos.y, pos.z & 15)); }
Block* LevelChunk::getExtraBlock(const BlockPos& pos) { return Block::byId(getExtraBlockId(pos.x & 15, pos.y, pos.z & 15)); }
const BlockState* LevelChunk::getBlockState(int x, int y, int z) { return Block::getStateByElementId((unsigned short)mElements->get(x, y, z, NULL)); }
const BlockState* LevelChunk::getBlockState(const BlockPos& pos) { return getBlockState(pos.x & 15, pos.y, pos.z & 15); }
const BlockState* LevelChunk::getExtraBlockState(const BlockPos& pos) { return Block::getStateByElementId((unsigned short)mElements->getExtraElement(pos.x & 15, pos.y, pos.z & 15)); }
void LevelChunk::getBlockAndExtraId_NoLock(int x, int y, int z, int* blockId, int* extraBlockId) { unsigned short element = 0; unsigned short extra = 0; mElements->getElementAndExtraElement_NoLock(x, y, z, &element, &extra); *blockId = element >> 4; *extraBlockId = extra >> 4; }
void LevelChunk::getElementReadLock() { mElements->getReadLockExternal(); }
void LevelChunk::releaseElementReadLock() { mElements->releaseReadLockExternal(); }
unsigned int LevelChunk::getElementFrameLastUpdated() { return mElements->getFrameLastUpdated(); }
unsigned short LevelChunk::setElement(int x, int y, int z, unsigned short element) { unsigned short old = mElements->get(x, y, z, NULL); mElements->set(x, y, z, element); return old; }
unsigned short LevelChunk::getChunkBlockData(int x, int y, int z) { return mElements->get(x, y, z, NULL); }
int LevelChunk::getData(const BlockPos& pos) { return getData(pos.x & 15, pos.y, pos.z & 15); }
bool LevelChunk::setData(int x, int y, int z, int data, int flags, bool* changed) { unsigned short old = mElements->get(x, y, z, NULL); mElements->set(x, y, z, (unsigned short)((old & 0xfff0) | (data & 15))); if (changed != NULL) *changed = old != mElements->get(x, y, z, NULL); (void)flags; return old != mElements->get(x, y, z, NULL); }
void LevelChunk::setBlock(const BlockPos& pos, const BlockState* state) { internalSetBlock(false, pos, state); }
void LevelChunk::setExtraBlock(const BlockPos& pos, const BlockState* state) { internalSetBlock(true, pos, state); }
bool LevelChunk::internalSetBlock(bool extra, const BlockPos& pos, const BlockState* state) { if (state == NULL) return false; unsigned short value = (unsigned short)state->mElementId; unsigned short old = extra ? mElements->getExtraElement(pos.x & 15, pos.y, pos.z & 15) : mElements->get(pos.x & 15, pos.y, pos.z & 15, NULL); if (extra) mElements->setExtraElement(pos.x & 15, pos.y, pos.z & 15, value); else mElements->set(pos.x & 15, pos.y, pos.z & 15, value); mUnsaved = true; return old != value; }
bool LevelChunk::setBlockAndData(int x, int y, int z, int blockId, int data, bool update, bool extra) { Block* block = Block::byId(blockId); if (block == NULL) return false; const BlockState* state = block->defaultBlockState(); bool changed = internalSetBlock(extra, BlockPos(this->x * 16 + x, y, this->z * 16 + z), state); (void)data; (void)update; return changed; }
void LevelChunk::setExtraElement(int x, int y, int z, unsigned short element) { mElements->setExtraElement(x, y, z, element); }
unsigned short LevelChunk::getExtraElement(int x, int y, int z) { return mElements->getExtraElement(x, y, z); }
void LevelChunk::getElementAndExtraElement(int x, int y, int z, unsigned short* element, unsigned short* extraElement) { mElements->getElementAndExtraElement(x, y, z, element, extraElement); }
int LevelChunk::getBrightness(LightLayer::variety layer, const BlockPos& pos) { SparseLightStorage* storage = layer == LightLayer::SKY ? (pos.y < 128 ? mSkyLight : mSkyLightUpper) : (pos.y < 128 ? mBlockLight : mBlockLightUpper); return storage == NULL ? 0 : storage->get(pos.x & 15, pos.y & 127, pos.z & 15); }
void LevelChunk::getNeighbourBrightnesses(int* brightnesses, LightLayer::variety layer, int x, int y, int z) { if (brightnesses == NULL) return; brightnesses[0] = getBrightness(layer, BlockPos(x - 1, y, z)); brightnesses[1] = getBrightness(layer, BlockPos(x + 1, y, z)); brightnesses[2] = getBrightness(layer, BlockPos(x, y - 1, z)); brightnesses[3] = getBrightness(layer, BlockPos(x, y + 1, z)); }
bool LevelChunk::setBrightness(LightLayer::variety layer, const BlockPos& pos, int brightness) { SparseLightStorage*& storage = layer == LightLayer::SKY ? (pos.y < 128 ? mSkyLight : mSkyLightUpper) : (pos.y < 128 ? mBlockLight : mBlockLightUpper); if (storage == NULL) storage = new SparseLightStorage(layer == LightLayer::SKY); storage->set(pos.x & 15, pos.y & 127, pos.z & 15, brightness); mUnsaved = true; return true; }
int LevelChunk::getRawBrightness(const BlockPos& pos, int skyDarken) { int brightness = getBrightness(LightLayer::SKY, pos) - skyDarken; int block = getBrightness(LightLayer::BLOCK, pos); return brightness > block ? brightness : block; }
void LevelChunk::addEntity(boost::shared_ptr<Entity> entity) { mEntities.push_back(entity); }
void LevelChunk::removeEntity(boost::shared_ptr<Entity> entity) { for (std::vector<boost::shared_ptr<Entity> >::iterator it = mEntities.begin(); it != mEntities.end(); ++it) if (it->get() == entity.get()) { mEntities.erase(it); break; } }
void LevelChunk::removeEntity(boost::shared_ptr<Entity> entity, int section) { (void)section; removeEntity(entity); }
bool LevelChunk::isSkyLit(const BlockPos& pos) { return getBrightness(LightLayer::SKY, pos) > 0; }
boost::shared_ptr<BlockEntity> LevelChunk::createBlockEntity(const BlockPos& pos) { return boost::shared_ptr<BlockEntity>(); }
boost::shared_ptr<BlockEntity> LevelChunk::getBlockEntity(const BlockPos& pos, EntityCreationType type) { BlockEntityMap::iterator it = mBlockEntityMap.find(pos); if (it != mBlockEntityMap.end()) return it->second; (void)type; return boost::shared_ptr<BlockEntity>(); }
void LevelChunk::addBlockEntity(boost::shared_ptr<BlockEntity> entity) { if (entity.get() == NULL) return; entity->setLevel(level); mBlockEntityMap[entity->getBlockPos()] = entity; }
void LevelChunk::setBlockEntity(const BlockPos& pos, boost::shared_ptr<BlockEntity> entity) { removeBlockEntity(pos); if (entity.get() != NULL) { entity->setLevel(level); mBlockEntityMap[pos] = entity; } }
void LevelChunk::removeBlockEntity(const BlockPos& pos) { mBlockEntityMap.erase(pos); }
bool LevelChunk::load(bool full) { mLoaded = true; mUnloaded = false; (void)full; return true; }
bool LevelChunk::unload(bool save, bool keepLoaded) { if (save) mUnsaved = false; mUnloaded = !keepLoaded; mLoaded = keepLoaded; return true; }
bool LevelChunk::containsPlayer() { return !mEntities.empty(); }
bool LevelChunk::isUnloaded() { return mUnloaded; }
void LevelChunk::markUnsaved() { mUnsaved = true; }
void LevelChunk::getEntities(boost::shared_ptr<Entity> except, const AABB* box, std::vector<boost::shared_ptr<Entity> >& entities, const Predicate<boost::shared_ptr<Entity> >* predicate) {}
void LevelChunk::getEntitiesOfClass(const std::type_info& type, const AABB* box, std::vector<boost::shared_ptr<Entity> >& entities, const Predicate<boost::shared_ptr<Entity> >* predicate, bool exact) {}
int LevelChunk::countEntities() { return (int)mEntities.size(); }
bool LevelChunk::shouldSave(bool force) { return force || mUnsaved || mLastSaveHadEntities; }
int LevelChunk::getBlocksAndData(arrayWithLength<unsigned char>* output, int x, int y, int z, int width, int height, int depth, int offset, bool includeLight) { int next = offset + mElements->getDataRegion(*output, x, y, z, width, height, depth, offset); if (includeLight) { if (y < 128) next += mBlockLight->getDataRegion(*output, x, y, z, width, height < 128 ? height : 128, depth, next); if (height > 128) next += mBlockLightUpper->getDataRegion(*output, x, y < 128 ? 0 : y - 128, z, width, height - 128, depth, next); if (y < 128) next += mSkyLight->getDataRegion(*output, x, y, z, width, height < 128 ? height : 128, depth, next); if (height > 128) next += mSkyLightUpper->getDataRegion(*output, x, y < 128 ? 0 : y - 128, z, width, height - 128, depth, next); } return next; }
void LevelChunk::blockUpdatedCallback(int x, int y, int z, void* data, int value) { LevelChunk* chunk = static_cast<LevelChunk*>(data); chunk->level->checkLight(BlockPos(chunk->x * 16 + x, y + value, chunk->z * 16 + z), false, false); }
int LevelChunk::setBlocksAndData(arrayWithLength<unsigned char> input, int x, int y, int z, int width, int height, int depth, int flags, bool extra) { return 0; }
Random LevelChunk::getRandom(long long seed) { long long chunkSeed = level->getSeed() + x * x * 0x4c1906LL + x * 0x5ac0dbLL + z * z * 0x4307a7LL + z * 0x5f24fLL; return Random(chunkSeed ^ seed); }
bool LevelChunk::isEmpty() { for (int section = 0; section < 16; ++section) { int highest = 0; if (!mElements->isRenderChunkEmpty(section * 16, &highest)) return false; } return true; }
void LevelChunk::staticCtor() { InitializeCriticalSection(&sLevelChunkLightingMutex); InitializeCriticalSection(&sLevelChunkEntityMutex); InitializeCriticalSection(&sLevelChunkPostProcessMutex); }
void LevelChunk::postProcess() { mPostProcess = 1; }
void LevelChunk::flagPostProcessComplete(short value) { mPostProcess = value != 0; }
void LevelChunk::doPostProcess(ChunkSource* source, ChunkGenerator* generator) {}
bool LevelChunk::checkPostProcess(ChunkSource* source, ChunkGenerator* generator) { return false; }
bool LevelChunk::checkChests(ChunkSource* source, int x, int z) { return false; }
void LevelChunk::tick(bool update) {}
ChunkPos LevelChunk::getPos() { return ChunkPos(x, z); }
bool LevelChunk::isYSpaceEmpty(int y0, int y1) { for (int y = y0; y < y1; y += 16) { int highest = 0; if (!mElements->isRenderChunkEmpty(y, &highest)) return false; } return true; }
bool LevelChunk::reloadBiomes() { updateBiomePresentFlags(); return true; }
Biome* LevelChunk::getBiome(const BlockPos& pos, BiomeSource* source) { return source == NULL ? NULL : source->getBiome(pos); }
arrayWithLength<unsigned char> LevelChunk::getBiomes() { return mBiomes; }
void LevelChunk::updateBiomePresentFlags() {}
void LevelChunk::setBiomes(arrayWithLength<unsigned char> biomes) { mBiomes = biomes; updateBiomePresentFlags(); }
void LevelChunk::setCheckAllLight() { mLightPopulated = 0; }
bool LevelChunk::postProcessAt(int x, int z) { (void)x; (void)z; mPostProcess = 1; return true; }
Level* LevelChunk::getLevel() { return level; }
arrayWithLength<unsigned char> LevelChunk::getHeightmap() { arrayWithLength<unsigned char> result(256, false); for (int i = 0; i < 256; ++i) result.data[i] = mHeightmap[i]; return result; }
void LevelChunk::recalcMinAndMaxHeights() { mMinHeight = 255; mMaxHeight = 0; for (int i = 0; i < 256; ++i) { if (mHeightmap[i] < mMinHeight) mMinHeight = mHeightmap[i]; if (mHeightmap[i] > mMaxHeight) mMaxHeight = mHeightmap[i]; } }
LevelChunk::BlockEntityMap* LevelChunk::getBlockEntities() { return &mBlockEntityMap; }
void LevelChunk::setLastSaveHadEntities(bool value) { mLastSaveHadEntities = value; }
void LevelChunk::setLastSaveTime(long long value) { (void)value; }
void LevelChunk::setInhabitedTime(long long value) { mInhabitedTime = value; }
void LevelChunk::compressLighting() { if (mSkyLight != NULL) mSkyLight->compress(); if (mSkyLightUpper != NULL) mSkyLightUpper->compress(); if (mBlockLight != NULL) mBlockLight->compress(); if (mBlockLightUpper != NULL) mBlockLightUpper->compress(); }
void LevelChunk::compressBlocks() { if (mElements != NULL) mElements->removeCompressionHeadroom(); }
void LevelChunk::compressData() {}
void LevelChunk::setChunkVersion(int version) { mChunkVersion = version; }
void LevelChunk::swapEntities(LevelChunk* chunk) { if (chunk != NULL) mEntities.swap(chunk->mEntities); }
int LevelChunk::PackBlockPos(const BlockPos& pos) { return (short)((pos.y & 255) | ((pos.z << 8) & 0xf00) | ((pos.x << 12) & 0xf000)); }
BlockPos LevelChunk::UnpackBlockPos(short packed) { unsigned short value = (unsigned short)packed; return BlockPos((value >> 12) & 15, value & 255, (value >> 8) & 15); }
void LevelChunk::CreateNewThreadStorage() { ThreadStorage* storage = new ThreadStorage(); storage->elementStorage = new unsigned short[0x10000]; storage->extraElementStorage = new unsigned short[0x10000]; storage->tempLightingStorage = new unsigned char[0x4080]; TlsSetValue(sLevelChunkThreadStorage, storage); }
void LevelChunk::ReleaseThreadStorage() { ThreadStorage* storage = static_cast<ThreadStorage*>(TlsGetValue(sLevelChunkThreadStorage)); delete[] storage->elementStorage; delete[] storage->extraElementStorage; delete[] storage->tempLightingStorage; delete storage; }
