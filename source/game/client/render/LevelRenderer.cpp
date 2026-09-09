#include "game/client/render/LevelRenderer.hpp"

#include <algorithm>
#include <cmath>

#include "game/client/audio/SoundEngine.hpp"
#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Gui.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"

unsigned int gChunkDimensionSizes[2] = { 0x19000, 0x1a900 };
unsigned int gChunkDimensionBases[2] = { 0, 0x19000 };
static const int gEndChunkCenterX = 0;
static const int gEndChunkCenterZ = 0;
static const int gEndChunkOuterX = 0;
static const int gEndChunkOuterZ = 25;
static const unsigned int gEndChunkData = 0x1a900;

int getChunkIndex(int dimension, int x, int y, int z)
{
    unsigned int dimensionSize;
    unsigned int xOffset;
    unsigned int zOffset;
    int centeredOffset;

    if (dimension < 2)
    {
        dimensionSize = gChunkDimensionSizes[dimension];
        centeredOffset = ((int)dimensionSize >> 1) +
                         ((int)dimensionSize < 0 && (dimensionSize & 1) != 0);
        z += centeredOffset;
        centeredOffset += x;
        if (centeredOffset >= 0 && centeredOffset < (int)dimensionSize &&
            y < 16 && z >= 0 && z < (int)dimensionSize)
            return (int)(gChunkDimensionBases[dimension] +
                         ((centeredOffset * dimensionSize + z) * 16) + y);
    }
    else
    {
        if (dimension != 2)
            return -1;

        xOffset = (unsigned int)((x + 9) - gEndChunkCenterX);
        zOffset = (unsigned int)((z + 9) - gEndChunkCenterZ);
        if (xOffset < 0x12 && y < 16 && zOffset < 0x12)
            return (int)(gEndChunkData + xOffset * 0x120 + zOffset * 16 + y);

        xOffset = (unsigned int)((x + 6) - gEndChunkOuterX);
        zOffset = (unsigned int)((z + 6) - gEndChunkOuterZ);
        if (xOffset < 0xc && y < 16 && zOffset < 0xc)
            return (int)(gEndChunkData + xOffset * 0xc0 + zOffset * 16 + y + 0x1440);
    }
    return -1;
}

unsigned int gGlobalChunkCount = 0x1c640;

LevelRenderer::DestroyedBlockManager::RecentBlock::RecentBlock(const BlockPos& blockPos, Level* sourceLevel)
    : pos(blockPos), level(sourceLevel), ticks(20), remove(false)
{
}

LevelRenderer::DestroyedBlockManager::RecentBlock::~RecentBlock()
{
    for (std::vector<AABB*>::iterator box = boxes.begin(); box != boxes.end(); ++box)
        delete *box;
}

LevelRenderer::DestroyedBlockManager::DestroyedBlockManager()
{
    InitializeCriticalSection(&lock);
}

LevelRenderer::DestroyedBlockManager::~DestroyedBlockManager()
{
    for (std::vector<RecentBlock*>::iterator block = blocks.begin(); block != blocks.end(); ++block)
        delete *block;
    DeleteCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::tick()
{
    EnterCriticalSection(&lock);
    std::vector<RecentBlock*>::iterator block = blocks.begin();
    while (block != blocks.end())
    {
        --(*block)->ticks;
        if ((*block)->ticks == 0)
        {
            delete *block;
            *block = blocks.back();
            blocks.pop_back();
        }
        else
        {
            ++block;
        }
    }
    LeaveCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::updatedChunkAt(Level* sourceLevel, const BlockPos& chunkPos, int radius)
{
    EnterCriticalSection(&lock);
    std::vector<RecentBlock*>::iterator block = blocks.begin();
    while (block != blocks.end())
    {
        RecentBlock* recent = *block;
        if (recent->level == sourceLevel &&
            recent->pos.x >= chunkPos.x && recent->pos.x < chunkPos.x + 16 &&
            recent->pos.y >= chunkPos.y && recent->pos.y < chunkPos.y + 16 &&
            recent->pos.z >= chunkPos.z && recent->pos.z < chunkPos.z + 16)
            recent->remove = true;
        ++block;
    }

    if (radius < 2)
    {
        block = blocks.begin();
        while (block != blocks.end())
        {
            if ((*block)->remove)
            {
                delete *block;
                *block = blocks.back();
                blocks.pop_back();
            }
            else
            {
                ++block;
            }
        }
    }
    LeaveCriticalSection(&lock);
}

bool LevelRenderer::sortEntDisFunc(std::pair<int, double> first, std::pair<int, double> second)
{
    return first.second < second.second;
}

int LevelRenderer::getDimensionIndexFromId(int id)
{
    return (3 - id) % 3;
}

bool LevelRenderer::isInCloud(double, double, double, float)
{
    return false;
}

void LevelRenderer::playSound(boost::shared_ptr<Player>, const SoundEvent*, SoundSource::ESoundSource,
                              double, double, double, float, float, float)
{
}

void LevelRenderer::playerRemoved(boost::shared_ptr<Entity>)
{
}

LevelRenderer::LevelRenderer(Minecraft* minecraft, Textures* textures)
    : mTextures(textures), mLastViewDistance(0), mSkyColorChanged(false), mDisplayList(0),
      mStarList(0), mSkyList(0), mMinecraft(minecraft), mEntityRenderDispatcher(0),
      mBlockEntityRenderDispatcher(0), mFont(0), mChunkGridWidth(0), mChunkGridHeight(0),
      mChunkGridDepth(0), mCameraX(0.0), mCameraY(0.0), mCameraZ(0.0), mLastCameraX(0.0),
      mLastCameraY(0.0), mLastCameraZ(0.0), mRenderTick(0), mDestroyedBlocks(0),
      mCloudOffset(0.0f), mGlobalChunkFlags(0)
{
    for (int dimension = 0; dimension < 4; ++dimension)
        mLevels[dimension] = 0;
    InitializeCriticalSection(&mBlockEntityMutex);
    InitializeCriticalSection(&mChunkMutex);
    InitializeCriticalSection(&mDirtyMutex);
    InitializeCriticalSection(&mGlobalChunkFlagsMutex);
    mEntityRenderDispatcher = minecraft->getEntityRenderDispatcher();
    mFont = minecraft->mFont;
    mDisplayList = GlStateManager::genLists(getGlobalChunkCount() * 3);
    mStarList = GlStateManager::genLists(4);
    mGlobalChunkFlags = new unsigned char[getGlobalChunkCount()]();
    mDestroyedBlocks = new DestroyedBlockManager();
    GlStateManager::newList(mStarList, 0);
    renderStars();
    GlStateManager::endList(0);
    createCloudMesh();
}

LevelRenderer::~LevelRenderer()
{
    for (std::vector<Chunk*>::iterator chunk = mChunks.begin(); chunk != mChunks.end(); ++chunk)
        delete *chunk;
    for (boost::unordered_map<int, BlockDestructionProgress*>::iterator progress = mDestroyingBlocks.begin(); progress != mDestroyingBlocks.end(); ++progress)
        delete progress->second;
    delete[] mGlobalChunkFlags;
    delete mDestroyedBlocks;
    DeleteCriticalSection(&mGlobalChunkFlagsMutex);
    DeleteCriticalSection(&mDirtyMutex);
    DeleteCriticalSection(&mChunkMutex);
    DeleteCriticalSection(&mBlockEntityMutex);
}

unsigned int LevelRenderer::getGlobalChunkCount()
{
    return gGlobalChunkCount;
}

void LevelRenderer::renderStars()
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Random random(10842);
    builder->begin();
    for (int star = 0; star < 1500; ++star)
    {
        float x = random.nextFloat() * 2.0f - 1.0f;
        float y = random.nextFloat() * 2.0f - 1.0f;
        float z = random.nextFloat() * 2.0f - 1.0f;
        float length = x * x + y * y + z * z;
        if (length < 1.0f && length > 0.01f)
        {
            float scale = 100.0f / std::sqrt(length);
            builder->vertex(x * scale, y * scale, z * scale);
        }
    }
    Tesselator::getInstance()->end();
}

void LevelRenderer::createCloudMesh()
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    mSkyList = mStarList + 1;
    GlStateManager::newList(mSkyList, 0);
    builder->begin();
    for (int x = -384; x <= 384; x += 64)
        for (int z = -384; z <= 384; z += 64)
        {
            builder->vertex((float)x, 16.0f, (float)z);
            builder->vertex((float)(x + 64), 16.0f, (float)z);
            builder->vertex((float)(x + 64), 16.0f, (float)(z + 64));
            builder->vertex((float)x, 16.0f, (float)(z + 64));
        }
    Tesselator::getInstance()->end();
    GlStateManager::endList(0);
}

unsigned char LevelRenderer::activePlayers()
{
    unsigned char count = 0;
    for (int index = 0; index < 4; ++index)
        if (mLevels[index] != 0)
            ++count;
    return count;
}

void LevelRenderer::nonStackDirtyChunksAdded()
{
    EnterCriticalSection(&mDirtyMutex);
    ++mRenderTick;
    LeaveCriticalSection(&mDirtyMutex);
}

void LevelRenderer::resortChunks(int x, int y, int z, int dimension)
{
    EnterCriticalSection(&mChunkMutex);
    for (std::vector<Chunk*>::iterator chunk = mChunks.begin(); chunk != mChunks.end(); ++chunk)
        if (dimension < 0 || (*chunk)->mDimension == dimension)
            (*chunk)->setPos(x, z, (*chunk)->mDimension);
    LeaveCriticalSection(&mChunkMutex);
}

void LevelRenderer::allChanged(int dimension)
{
    EnterCriticalSection(&mChunkMutex);
    for (std::vector<Chunk*>::iterator chunk = mChunks.begin(); chunk != mChunks.end(); ++chunk)
        if (dimension < 0 || (*chunk)->mDimension == dimension)
            (*chunk)->setDirty();
    LeaveCriticalSection(&mChunkMutex);
    nonStackDirtyChunksAdded();
}

void LevelRenderer::setLevel(int dimension, MultiPlayerLevel* level)
{
    int index = getDimensionIndexFromId(dimension);
    if (index >= 0 && index < 4)
        mLevels[index] = level;
    if (mBlockEntityRenderDispatcher != 0)
        mBlockEntityRenderDispatcher->setLevel(level);
    if (mMinecraft->mParticleEngine != 0)
        mMinecraft->mParticleEngine->setLevel(level);
    allChanged(dimension);
}

void LevelRenderer::tick()
{
    ++mRenderTick;
    mCloudOffset += 1.0f;
    if (mDestroyedBlocks != 0)
        mDestroyedBlocks->tick();
}

void LevelRenderer::clear()
{
    allChanged(-1);
}

void LevelRenderer::allChanged()
{
    allChanged(-1);
}

bool LevelRenderer::shouldEntityGlow(boost::shared_ptr<Entity> entity, boost::shared_ptr<Entity> camera, Culler* culler)
{
    if (!entity || entity == camera)
        return false;
    return mEntityRenderDispatcher->shouldRender(entity, culler, entity->mX - mCameraX, entity->mY - mCameraY, entity->mZ - mCameraZ);
}

bool LevelRenderer::isGlobalIndexInSameDimension(int index, Level* level)
{
    if (index < 0 || level == 0 || level->mDimension == 0)
        return false;
    return index / (int)(getGlobalChunkCount() / 3) == getDimensionIndexFromId(level->mDimension->getType()->getId());
}

void LevelRenderer::setupDestroyState()
{
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(770, 1);
    GlStateManager::enablePolygonOffset();
    GlStateManager::polygonOffset(-3.0f, -3.0f);
}

void LevelRenderer::restoreDestroyState()
{
    GlStateManager::polygonOffset(0.0f, 0.0f);
    GlStateManager::disablePolygonOffset();
    GlStateManager::disableBlend();
}

void LevelRenderer::renderEntities(boost::shared_ptr<Entity> camera, Culler* culler, float partialTick)
{
    Level* level = mLevels[getDimensionIndexFromId(camera->mDimension)];
    mEntityRenderDispatcher->prepare(level, mFont, camera, mMinecraft->getCameraEntity(), mMinecraft->mOptions, partialTick);
    std::vector<boost::shared_ptr<Entity> > entities;
    AABB* bounds = AABB::newTemp(mCameraX - 4096.0, mCameraY - 4096.0, mCameraZ - 4096.0,
                                 mCameraX + 4096.0, mCameraY + 4096.0, mCameraZ + 4096.0);
    level->getEntities(boost::shared_ptr<Entity>(), bounds, &entities);
    std::vector<boost::shared_ptr<Entity> >::iterator entity = entities.begin();
    for (; entity != entities.end(); ++entity)
        if (*entity && mEntityRenderDispatcher->shouldRender(*entity, culler, (*entity)->mX - mCameraX, (*entity)->mY - mCameraY, (*entity)->mZ - mCameraZ))
            mEntityRenderDispatcher->render(*entity, partialTick, true);
}

unsigned int LevelRenderer::getBucketMask(double x, double y, double z, ClipChunk*)
{
    unsigned int mask = 0;
    if (x < 0.0) mask |= 1;
    if (y < 0.0) mask |= 2;
    if (z < 0.0) mask |= 4;
    return mask;
}

void LevelRenderer::setupCameraInsideAreas(boost::shared_ptr<Entity> camera)
{
    mCameraX = camera->mX;
    mCameraY = camera->mY;
    mCameraZ = camera->mZ;
}

unsigned int LevelRenderer::clippedByCameraInsideAreas(ClipChunk* chunk)
{
    return getBucketMask(mCameraX, mCameraY, mCameraZ, chunk);
}

void LevelRenderer::renderChunks(int pass, int, int count, double)
{
    int rendered = 0;
    for (std::vector<Chunk*>::iterator chunk = mChunks.begin(); chunk != mChunks.end() && rendered < count; ++chunk)
        if (!(*chunk)->isEmpty() && ((*chunk)->mRenderable & (1 << pass)) != 0)
        {
            GlStateManager::callList(mDisplayList + rendered * 3 + pass);
            ++rendered;
        }
}

void LevelRenderer::render(boost::shared_ptr<Entity> camera, int pass, double distance, bool translucent)
{
    setupCameraInsideAreas(camera);
    if (translucent)
        GlStateManager::enableBlend();
    renderChunks(pass, 0, (int)mChunks.size(), distance);
    if (translucent)
        GlStateManager::disableBlend();
    mLastCameraX = mCameraX;
    mLastCameraY = mCameraY;
    mLastCameraZ = mCameraZ;
}

void LevelRenderer::renderSky(float)
{
    GlStateManager::disableTexture();
    GlStateManager::callList(mStarList);
    GlStateManager::enableTexture();
}

void LevelRenderer::renderHaloRing(float partialTick)
{
    GlStateManager::pushMatrix();
    GlStateManager::rotatef(partialTick, 0.0f, 1.0f, 0.0f);
    GlStateManager::callList(mSkyList);
    GlStateManager::popMatrix();
}

void LevelRenderer::renderAdvancedClouds(float partialTick, int pass, double x, double y, double z)
{
    GlStateManager::pushMatrix();
    GlStateManager::translated(-x, -y, -z);
    GlStateManager::callList(mSkyList + pass);
    GlStateManager::popMatrix();
    mCloudOffset += partialTick;
}

void LevelRenderer::renderClouds(float partialTick, int pass, double x, double y, double z)
{
    renderAdvancedClouds(partialTick, pass, x, y, z);
}

void LevelRenderer::setGlobalChunkFlag(int index, unsigned char mask, unsigned char shift)
{
    if (index >= 0 && (unsigned int)index < getGlobalChunkCount())
        mGlobalChunkFlags[index] |= (unsigned char)(mask << shift);
}

unsigned long long LevelRenderer::getGlobalIndexForChunk(int x, int y, int z, Level* level)
{
    if (level == 0 || level->mDimension == 0)
        return (unsigned long long)-1;
    return getChunkIndex(getDimensionIndexFromId(level->mDimension->getType()->getId()), x >> 4, y >> 4, z >> 4);
}

bool LevelRenderer::getGlobalChunkFlag(int x, int y, int z, Level* level, unsigned char mask, unsigned char shift)
{
    unsigned long long index = getGlobalIndexForChunk(x, y, z, level);
    return index != (unsigned long long)-1 && (mGlobalChunkFlags[index] & (mask << shift)) != 0;
}

unsigned int LevelRenderer::updateDirtyChunks(unsigned int* count)
{
    unsigned int rebuilt = 0;
    for (std::vector<Chunk*>::iterator chunk = mChunks.begin(); chunk != mChunks.end(); ++chunk)
        if ((*chunk)->mDirty != 0)
        {
            (*chunk)->rebuild(count);
            ++rebuilt;
        }
    return rebuilt;
}

void LevelRenderer::renderHit(boost::shared_ptr<Player>, const HitResult* hit, int, not_null_ptr<ItemInstance>, float)
{
    if (hit != 0 && hit->mType == HitResult::BLOCK)
        renderLineBox(AABB::newTemp(hit->getBlockPos()), 0xffffffff);
}

void LevelRenderer::renderDestroyAnimation(Tesselator*, boost::shared_ptr<Player>, float)
{
    setupDestroyState();
    for (boost::unordered_map<int, BlockDestructionProgress*>::iterator progress = mDestroyingBlocks.begin(); progress != mDestroyingBlocks.end(); ++progress)
        renderLineBox(AABB::newTemp(progress->second->getPos()), progress->second->getProgress());
    restoreDestroyState();
}

void LevelRenderer::renderLineBox(const AABB* box, int color)
{
    renderColoredLineBox(box, color, true);
}

void LevelRenderer::renderHitOutline(boost::shared_ptr<Player>, const HitResult* hit, int color, float)
{
    if (hit != 0 && hit->mType == HitResult::BLOCK)
        renderColoredLineBox(AABB::newTemp(hit->getBlockPos()), color, false);
}

void LevelRenderer::renderColoredLineBox(const AABB* box, int color, bool depth)
{
    if (!depth)
        GlStateManager::disableDepthTest();
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin(1);
    builder->color(color);
    builder->vertexDouble(box->mMinX, box->mMinY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMinY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMinY, box->mMaxZ);
    builder->vertexDouble(box->mMinX, box->mMinY, box->mMaxZ);
    builder->vertexDouble(box->mMinX, box->mMaxY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMaxY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMaxY, box->mMaxZ);
    builder->vertexDouble(box->mMinX, box->mMaxY, box->mMaxZ);
    Tesselator::getInstance()->end();
    if (!depth)
        GlStateManager::enableDepthTest();
}

void LevelRenderer::renderLine(std::vector<GameRenderer::PositionStruct>* positions, std::vector<int>* colors, std::vector<int>* indices, float width)
{
    GlStateManager::lineWidth(width);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin(1);
    for (std::vector<int>::iterator index = indices->begin(); index != indices->end(); ++index)
    {
        int vertex = *index;
        builder->color((*colors)[vertex]);
        builder->vertex(positions->at(vertex).x, positions->at(vertex).y, positions->at(vertex).z);
    }
    Tesselator::getInstance()->end();
}

void LevelRenderer::renderColoredBox(const AABB* box, int color, int alpha)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    builder->color(color, alpha);
    builder->vertexDouble(box->mMinX, box->mMinY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMinY, box->mMinZ);
    builder->vertexDouble(box->mMaxX, box->mMaxY, box->mMaxZ);
    builder->vertexDouble(box->mMinX, box->mMaxY, box->mMaxZ);
    Tesselator::getInstance()->end();
}

void LevelRenderer::renderStringInWorld(boost::shared_ptr<Entity> entity, const std::wstring& text, Vec3* position)
{
    GameRenderer::renderNameTagInWorld(Minecraft::GetInstance()->mFont, text, (float)(position->x - entity->mX),
                                       (float)(position->y - entity->mY), (float)(position->z - entity->mZ),
                                       0xffffffff, 1.0f, 0.02666667f, false, false, 0, 1.0f);
}

void LevelRenderer::renderStringOnPlane(boost::shared_ptr<Entity> entity, const std::wstring& text, Vec3* origin, Vec3*)
{
    renderStringInWorld(entity, text, origin);
}

void LevelRenderer::renderDebugVolume(boost::shared_ptr<Entity> entity, const std::wstring& text, const AABB* box, int color,
                                      bool drawText, bool drawSolid, bool drawWireframe, bool depth)
{
    if (drawSolid)
        renderColoredBox(box, color, 64);
    if (drawWireframe)
        renderColoredLineBox(box, color, depth);
    if (drawText)
        renderStringInWorld(entity, text, box->getCenter());
}

void LevelRenderer::renderFancyTextDebugVolume(boost::shared_ptr<Entity> entity, const std::wstring& text, const AABB* box,
                                               int color, bool drawSolid, bool drawWireframe, bool depth)
{
    renderDebugVolume(entity, text, box, color, true, drawSolid, drawWireframe, depth);
}

void LevelRenderer::renderDebugWireframeVolume(boost::shared_ptr<Entity> entity, const std::wstring& text, const AABB* box,
                                               int color, bool drawText, bool drawSolid, bool drawWireframe, bool depth)
{
    renderDebugVolume(entity, text, box, color, drawText, drawSolid, drawWireframe, depth);
}

void LevelRenderer::setDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, Level* level)
{
    EnterCriticalSection(&mChunkMutex);
    for (std::vector<Chunk*>::iterator chunk = mChunks.begin(); chunk != mChunks.end(); ++chunk)
        if ((*chunk)->mLevel == level && (*chunk)->mX >= minX - 16 && (*chunk)->mX <= maxX + 16 &&
            (*chunk)->mZ >= minZ - 16 && (*chunk)->mZ <= maxZ + 16 &&
            (*chunk)->mDimension >= minY - 16 && (*chunk)->mDimension <= maxY + 16)
            (*chunk)->setDirty();
    LeaveCriticalSection(&mChunkMutex);
}

void LevelRenderer::blockChanged(Level* level, const BlockPos& pos, const BlockState*, const BlockState*, int, bool immediate)
{
    setDirty(pos.x - 1, pos.y - 1, pos.z - 1, pos.x + 1, pos.y + 1, pos.z + 1, level);
    notifyNearbyEntities(level, pos, immediate);
}

void LevelRenderer::blockLightChanged(const BlockPos& pos)
{
    Level* level = mLevels[0];
    setDirty(pos.x - 1, pos.y - 1, pos.z - 1, pos.x + 1, pos.y + 1, pos.z + 1, level);
}

void LevelRenderer::setBlocksDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, Level* level)
{
    setDirty(minX, minY, minZ, maxX, maxY, maxZ, level);
}

void LevelRenderer::cull(Culler* culler, float)
{
    for (std::vector<Chunk*>::iterator chunk = mChunks.begin(); chunk != mChunks.end(); ++chunk)
        (*chunk)->cull(culler);
}

void LevelRenderer::notifyNearbyEntities(Level* level, const BlockPos& pos, bool)
{
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntities(boost::shared_ptr<Entity>(), AABB::newTemp(pos), 0);
    for (std::vector<boost::shared_ptr<Entity> >::iterator entity = entities->begin(); entity != entities->end(); ++entity)
        if (*entity)
            (*entity)->setPos((*entity)->mX, (*entity)->mY, (*entity)->mZ);
}

void LevelRenderer::playStreamingMusic(const SoundEvent* sound, const BlockPos& pos, bool playing)
{
    if (mMinecraft->mSoundEngine != 0)
        mMinecraft->mSoundEngine->playStreaming(sound, (float)pos.x, (float)pos.y, (float)pos.z, 1.0f, 1.0f,
                                                playing, false, ConsoleSoundEngine::eAudioDelayType_None);
}

void LevelRenderer::addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z,
                                double xa, double ya, double za, arrayWithLength<int> params)
{
    addParticleInternal(level, type, overrideLimiter, x, y, z, xa, ya, za, params);
}

int LevelRenderer::calculateParticleLevel(Level*, bool overrideLimiter)
{
    return overrideLimiter ? 0 : 1;
}

void LevelRenderer::addParticleInternal(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z,
                                        double xa, double ya, double za, arrayWithLength<int> params)
{
    addParticleInternal(level, type, overrideLimiter, false, x, y, z, xa, ya, za, params);
}

void LevelRenderer::addParticleInternal(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, bool force,
                                        double x, double y, double z, double xa, double ya, double za,
                                        arrayWithLength<int> params)
{
    if ((force || calculateParticleLevel(level, overrideLimiter) < 2) && mMinecraft->mParticleEngine != 0)
    {
        Particle* particle = mMinecraft->mParticleEngine->createParticle(type, x, y, z, xa, ya, za, params, level);
        if (particle != 0)
            mMinecraft->mParticleEngine->add(particle);
    }
}

void LevelRenderer::addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, bool force, double x, double y, double z,
                                double xa, double ya, double za, arrayWithLength<int> params)
{
    addParticleInternal(level, type, overrideLimiter, force, x, y, z, xa, ya, za, params);
}

void LevelRenderer::addParticle(Level* level, const ParticleType* particle, double x, double y, double z,
                                double xa, double ya, double za, arrayWithLength<int> params)
{
    level->addParticle(particle, x, y, z, xa, ya, za, params);
}

void LevelRenderer::entityAdded(boost::shared_ptr<Entity> entity)
{
    if (entity)
        entity->setPos(entity->mX, entity->mY, entity->mZ);
}

void LevelRenderer::entityRemoved(boost::shared_ptr<Entity> entity)
{
    if (entity)
        entity->setPos(entity->mX, entity->mY, entity->mZ);
}

void LevelRenderer::globalLevelEvent(int type, const BlockPos& pos, int data)
{
    levelEvent(boost::shared_ptr<Player>(), type, pos, data);
}

void LevelRenderer::levelEvent(boost::shared_ptr<Player>, int type, const BlockPos& pos, int data)
{
    Level* level = mLevels[0];
    if (level == 0)
        return;
    arrayWithLength<int> params(1, false);
    params.data[0] = data;
    if (type >= 2000)
        addParticle(level, (ePARTICLE_TYPE)data, true, (double)pos.x + 0.5, (double)pos.y + 0.5, (double)pos.z + 0.5,
                    0.0, 0.0, 0.0, params);
}

void LevelRenderer::destroyBlockProgress(int breakerId, const BlockPos& pos, int progress)
{
    boost::unordered_map<int, BlockDestructionProgress*>::iterator current = mDestroyingBlocks.find(breakerId);
    if (progress < 0 || progress > 9)
    {
        if (current != mDestroyingBlocks.end())
        {
            delete current->second;
            mDestroyingBlocks.erase(current);
        }
        return;
    }
    if (current == mDestroyingBlocks.end())
        current = mDestroyingBlocks.insert(std::make_pair(breakerId, new BlockDestructionProgress(breakerId, pos))).first;
    current->second->setProgress(progress);
    current->second->updateTick(mRenderTick);
}

void LevelRenderer::registerTextures(IconRegister*)
{
    mSkyColorChanged = true;
}

void LevelRenderer::fullyFlagRenderableBlockEntitiesToBeRemoved()
{
    for (boost::unordered_set<boost::shared_ptr<BlockEntity> >::iterator block = mRenderableBlockEntities.begin(); block != mRenderableBlockEntities.end(); ++block)
        (*block)->setRenderRemoveStage(2);
}

unsigned long long LevelRenderer::getGlobalIndexForChunk(int x, int y, int z, int dimension)
{
    return getChunkIndex(getDimensionIndexFromId(dimension), x >> 4, y >> 4, z >> 4);
}

void LevelRenderer::setGlobalChunkFlag(int x, int y, int z, Level* level, unsigned char mask, unsigned char shift)
{
    unsigned long long index = getGlobalIndexForChunk(x, y, z, level);
    if (index != (unsigned long long)-1)
        mGlobalChunkFlags[index] |= (unsigned char)(mask << shift);
}

void LevelRenderer::clearGlobalChunkFlag(int x, int y, int z, Level* level, unsigned char mask, unsigned char shift)
{
    unsigned long long index = getGlobalIndexForChunk(x, y, z, level);
    if (index != (unsigned long long)-1)
        mGlobalChunkFlags[index] &= (unsigned char)~(mask << shift);
}

unsigned char LevelRenderer::getGlobalChunkFlags(int x, int y, int z, Level* level)
{
    unsigned long long index = getGlobalIndexForChunk(x, y, z, level);
    return index == (unsigned long long)-1 ? 0 : mGlobalChunkFlags[index];
}

unsigned long long LevelRenderer::setGlobalChunkFlags(int x, int y, int z, Level* level, unsigned char flags)
{
    unsigned long long index = getGlobalIndexForChunk(x, y, z, level);
    if (index != (unsigned long long)-1)
        mGlobalChunkFlags[index] = flags;
    return index;
}

unsigned char LevelRenderer::incGlobalChunkRefCount(int x, int y, int z, Level* level)
{
    unsigned long long index = getGlobalIndexForChunk(x, y, z, level);
    if (index == (unsigned long long)-1)
        return 0;
    unsigned char flags = mGlobalChunkFlags[index];
    unsigned char count = (unsigned char)((flags >> 5) + 1);
    mGlobalChunkFlags[index] = (unsigned char)((count << 5) | (flags & 31));
    return count;
}

unsigned char LevelRenderer::decGlobalChunkRefCount(int x, int y, int z, Level* level)
{
    unsigned long long index = getGlobalIndexForChunk(x, y, z, level);
    if (index == (unsigned long long)-1)
        return 0;
    unsigned char flags = mGlobalChunkFlags[index];
    unsigned char count = (unsigned char)((flags >> 5) - 1);
    mGlobalChunkFlags[index] = (unsigned char)((count << 5) | (flags & 31));
    return count;
}

int LevelRenderer::checkAllPresentChunks(bool* present)
{
    int count = 0;
    *present = true;
    for (std::vector<Chunk*>::iterator chunk = mChunks.begin(); chunk != mChunks.end(); ++chunk)
        if ((*chunk)->mLevel != 0)
            ++count;
        else
            *present = false;
    return count;
}

void LevelRenderer::DestroyedBlockManager::destroyingBlockAt(Level* level, const BlockPos& pos)
{
    EnterCriticalSection(&lock);
    RecentBlock* recent = new RecentBlock(pos, level);
    AABB* blockBounds = AABB::newTemp((double)pos.x, (double)pos.y, (double)pos.z,
                                      (double)pos.x + 1.0, (double)pos.y + 1.0, (double)pos.z + 1.0);
    const BlockState* state = level->getBlockState(pos);
    state->getBlock()->addCollisionAABBs(state, level, pos, blockBounds, &recent->boxes,
                                         boost::shared_ptr<Entity>(), false);
    for (std::vector<AABB*>::iterator box = recent->boxes.begin(); box != recent->boxes.end(); ++box)
        *box = AABB::newPermanent(*box);
    blocks.push_back(recent);
    LeaveCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::addAABBs(Level* level, const AABB* area, std::vector<AABB*>* result)
{
    EnterCriticalSection(&lock);
    for (std::vector<RecentBlock*>::iterator block = blocks.begin(); block != blocks.end(); ++block)
        if ((*block)->level == level)
            for (std::vector<AABB*>::iterator box = (*block)->boxes.begin(); box != (*block)->boxes.end(); ++box)
                if ((*box)->intersects(area->mMinX, area->mMinY, area->mMinZ, area->mMaxX, area->mMaxY, area->mMaxZ))
                    result->push_back(AABB::newTemp(*box));
    LeaveCriticalSection(&lock);
}
