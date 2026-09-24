#include "game/world/chunk/Chunk.hpp"

#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Culler.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Region.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/GlideRingGenerator.hpp"

#include <algorithm>

void PIXSetMarkerDeprecated(float colour, const char* name, ...);
extern LevelRenderer* gLevelRenderer;

static LevelRenderer* chunkLevelRenderer()
{
    return gLevelRenderer;
}

Chunk::Chunk(Level* level, ChunkBlockEntityMap& blockEntities, OSFastMutex& mutex, int x, int z, int dimension, ClipChunk* clipChunk)
    : mLevel(level), mX(0), mZ(0), mDimension(0), mDirty(0), mEmpty(0), mRenderable(0),
      mMinX(0), mMinZ(0), mMinDimension(0), mMaxX(0), mMaxZ(0), mMaxDimension(0),
      mBoundingBox(NULL), mOwnsBoundingBox(false), mClipChunk(clipChunk), mUnknown(0),
      mBlockEntities(&blockEntities), mMutex(&mutex), mNonStack(false)
{
    mClipChunk->mVisible = false;
    mClipChunk->mChunk = this;
    setPos(x, z, dimension);
    mNonStack = true;
}

Chunk::Chunk()
    : mLevel(NULL), mX(0), mZ(0), mDimension(0), mDirty(0), mEmpty(0), mRenderable(0),
      mMinX(0), mMinZ(0), mMinDimension(0), mMaxX(0), mMaxZ(0), mMaxDimension(0),
      mBoundingBox(NULL), mOwnsBoundingBox(false), mClipChunk(NULL), mUnknown(0),
      mBlockEntities(NULL), mMutex(NULL), mNonStack(false)
{
}

Chunk::~Chunk()
{
    if (mOwnsBoundingBox)
        delete mBoundingBox;
}

void Chunk::reset()
{
    if (mNonStack)
    {
        LevelRenderer* renderer = chunkLevelRenderer();
        if (renderer != NULL)
        {
            EnterCriticalSection(&renderer->mChunkMutex);
            unsigned char references = renderer->decGlobalChunkRefCount(mX, mZ, mDimension, mLevel);
            mNonStack = false;
            if (references == 0)
            {
                int index = LevelRenderer::getGlobalIndexForChunk(mX, mZ, mDimension, mLevel);
                if (index >= 0)
                {
                    gRenderer->CBuffClear(renderer->mDisplayList + index * 3);
                    gRenderer->CBuffClear(renderer->mDisplayList + index * 3 + 1);
                    gRenderer->CBuffClear(renderer->mDisplayList + index * 3 + 2);
                    renderer->setGlobalChunkFlags(mX, mZ, mDimension, mLevel, 0);
                }
            }
            LeaveCriticalSection(&renderer->mChunkMutex);
        }
    }
    mClipChunk->mVisible = false;
}

void Chunk::setPos(int x, int z, int dimension)
{
    if (!mNonStack || x != mX || z != mZ || dimension != mDimension)
    {
        reset();
        mX = x;
        mZ = z;
        mDimension = dimension;
        mMinX = x;
        mMinZ = z;
        mMinDimension = dimension;
        mMaxX = x + 8;
        mMaxZ = z + 8;
        mMaxDimension = dimension + 8;
        mDirty = 0;
        mEmpty = 0;
        mRenderable = 0;
        mClipChunk->mX = x + 8;
        mClipChunk->mY = z + 8;
        mClipChunk->mZ = dimension + 8;
        mClipChunk->mGlobalIndex = (int)LevelRenderer::getGlobalIndexForChunk(x, z, dimension, mLevel);
        if (mBoundingBox == NULL)
        {
            mBoundingBox = AABB::newPermanent(-6.0, -6.0, -6.0, 22.0, 22.0, 22.0);
            mOwnsBoundingBox = true;
        }
        mClipChunk->mMinX = (float)(mBoundingBox->mMinX + x);
        mClipChunk->mMinY = (float)(mBoundingBox->mMinY + z);
        mClipChunk->mMinZ = (float)(mBoundingBox->mMinZ + dimension);
        mClipChunk->mMaxX = (float)(mBoundingBox->mMaxX + x);
        mClipChunk->mMaxY = (float)(mBoundingBox->mMaxY + z);
        mClipChunk->mMaxZ = (float)(mBoundingBox->mMaxZ + dimension);
        mNonStack = true;
        LevelRenderer* renderer = chunkLevelRenderer();
        if (renderer != NULL)
        {
            EnterCriticalSection(&renderer->mChunkMutex);
            if (renderer->incGlobalChunkRefCount(x, z, dimension, mLevel) == 1)
                renderer->setGlobalChunkFlag(x, z, dimension, mLevel, 2, 0);
            LeaveCriticalSection(&renderer->mChunkMutex);
        }
    }
}

void Chunk::translateToPos()
{
    GlStateManager::translatef((float)mMinX, (float)mMinZ, (float)mMinDimension);
}

unsigned int Chunk::elementToBlockId(unsigned short element)
{
    return (element >> 4) & 0xffff;
}

bool Chunk::isEmpty()
{
    return mEmpty != 0;
}

void Chunk::setDirty()
{
    mDirty = 1;
}

void Chunk::makeCopyForRebuild(Chunk* source)
{
    mX = source->mX;
    mZ = source->mZ;
    mDimension = source->mDimension;
    mDirty = source->mDirty;
    mEmpty = source->mEmpty;
    mRenderable = source->mRenderable;
    mMinX = source->mMinX;
    mMinZ = source->mMinZ;
    mMinDimension = source->mMinDimension;
    mMaxX = source->mMaxX;
    mMaxZ = source->mMaxZ;
    mMaxDimension = source->mMaxDimension;
    mBoundingBox = source->mBoundingBox;
    mClipChunk = source->mClipChunk;
    mUnknown = source->mUnknown;
    mBlockEntities = source->mBlockEntities;
    mMutex = source->mMutex;
    mLevel = source->mLevel;
}

void Chunk::rebuild(unsigned int* count)
{
    LevelRenderer* levelRenderer = chunkLevelRenderer();
    int globalIndex = LevelRenderer::getGlobalIndexForChunk(mX, mZ, mDimension, mLevel);
    if (globalIndex < 0)
        return;

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    int listBase = levelRenderer->mDisplayList + globalIndex * 3;

    static unsigned short cachedElements[65536];
    static unsigned short cachedExtraElements[65536];
    static const BlockState* cachedStates[18 * 16 * 16];
    arrayWithLength<unsigned short> elements(cachedElements, 65536);
    arrayWithLength<unsigned short> extraElements(cachedExtraElements, 65536);

    int firstSlice = std::max(0, (mZ >> 2) - 1);
    int lastSlice = std::min(64, ((mZ + 16) >> 2) + 1);
    LevelChunk* levelChunk = mLevel->getChunkAt(BlockPos(mX, mZ, mDimension));
    levelChunk->getElementRange(elements, extraElements, firstSlice, lastSlice);
    Region region(mLevel, BlockPos(mX - 1, mZ - 1, mDimension - 1),
                  BlockPos(mX + 17, mZ + 17, mDimension + 17), 1);
    region.setCachedBlocksAndData(cachedElements, mX >> 4, mZ >> 4, mDimension >> 4, cachedStates);
    BlockRenderer blockRenderer(&region, mX, mZ, mDimension, count);

    bool allEmpty = true;
    for (int x = 0; x < 16; ++x)
        for (int z = 0; z < 16; ++z)
            for (int y = std::max(0, mZ - 1); y < std::min(256, mZ + 17); ++y)
            {
                unsigned int index = (x << 11) | (z << 7) | (y & 0x7f) | ((y & 0x80) << 8);
                unsigned short element = cachedElements[index];
                unsigned int id = elementToBlockId(element);
                if (id == 1 || id == 2 || id == 3 || id == 7 ||
                    (id >= 12 && id <= 16) || id == 21 || id == 56 || id == 73 || id == 153)
                    cachedElements[index] = element | 0x8000;
                if (y >= mZ && y < mZ + 16 && element != 0)
                    allEmpty = false;
            }

    if (!allEmpty)
        for (int x = 1; x < 15; ++x)
            for (int z = 1; z < 15; ++z)
                for (int y = std::max(0, mZ); y < std::min(255, mZ + 16); ++y)
                {
                    unsigned int index = (x << 11) | (z << 7) | (y & 0x7f) | ((y & 0x80) << 8);
                    unsigned int above = (x << 11) | (z << 7) | ((y + 1) & 0x7f) | (((y + 1) & 0x80) << 8);
                    if ((cachedElements[index - 0x800] & 0x8000) &&
                        (cachedElements[index + 0x800] & 0x8000) &&
                        (cachedElements[index - 0x80] & 0x8000) &&
                        (cachedElements[index + 0x80] & 0x8000) &&
                        (cachedElements[above] & 0x8000) &&
                        (y == 0 || (cachedElements[(x << 11) | (z << 7) | ((y - 1) & 0x7f) |
                                                 (((y - 1) & 0x80) << 8)] & 0x8000)))
                        cachedElements[index] = 0xffff;
                }

    std::vector<BlockPos> ringPositions;
    std::vector<GlideRingGenerator::eGlideRingSize> ringSizes;
    if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
    {
        LevelRuleset* ruleset = g_pApp->getGameRuleDefinitions();
        std::vector<TargetAreaRuleDefinition*> rings;
        AABB* chunkArea = AABB::newTemp(mX, mZ, mDimension,
                                        mX + 16, mZ + 16, mDimension + 16);
        if (ruleset != NULL)
            ruleset->getRingsWithinArea(&rings, chunkArea);
        for (std::vector<TargetAreaRuleDefinition*>::iterator it = rings.begin(); it != rings.end(); ++it)
        {
            TargetAreaRuleDefinition* ring = *it;
            std::vector<BlockPos> positions;
            GlideRingGenerator::eGlideRingSize size =
                static_cast<GlideRingGenerator::eGlideRingSize>(ring->getSize());
            GlideRingGenerator::BuildRing(positions, ring->getArea(), size);
            for (std::vector<BlockPos>::iterator pos = positions.begin(); pos != positions.end(); ++pos)
                if (chunkArea->containsIncludingLowerBound(Vec3::newTemp(pos->x, pos->y, pos->z)))
                {
                    ringPositions.push_back(*pos);
                    ringSizes.push_back(size);
                }
        }
        for (std::vector<BlockPos>::iterator pos = ringPositions.begin(); pos != ringPositions.end(); ++pos)
        {
            int y = pos->y;
            unsigned int index = ((pos->x - mX) << 11) | ((pos->z - mDimension) << 7) |
                                 (y & 0x7f) | ((y & 0x80) << 8);
            cachedElements[index] = 0xffff;
        }
    }

    std::vector<boost::shared_ptr<BlockEntity> > renderedBlockEntities;
    if (allEmpty && ringPositions.empty())
    {
        for (int pass = 0; pass < 3; ++pass)
        {
            levelRenderer->setGlobalChunkFlag(mX, mZ, mDimension, mLevel, 4, pass);
            gRenderer->CBuffClear(listBase + pass);
        }
    }
    else
    {
        BufferBuilder::Bounds bounds;
        bounds.mMinX = bounds.mMinY = bounds.mMinZ = 22.0f;
        bounds.mMaxX = bounds.mMaxY = bounds.mMaxZ = -6.0f;
        for (int pass = 0; pass < 3; ++pass)
        {
            bool started = false;
            bool hasOtherLayer = false;
            bool rendered = false;
            if (pass == 0 && !ringPositions.empty())
                hasOtherLayer = true;
            if (pass == 1 && !ringPositions.empty())
            {
                MemSect(0x1f);
                GlStateManager::newList(listBase + pass, 0);
                MemSect(0);
                GlStateManager::pushMatrix();
                GlStateManager::depthMask(true);
                builder->useCompactVertices(true);
                translateToPos();
                builder->begin();
                builder->offset((float)-mX, (float)-mZ, (float)-mDimension);
                started = true;
                for (unsigned int ring = 0; ring < ringPositions.size(); ++ring)
                {
                    blockRenderer.setLightOverride(0x7fffffff);
                    blockRenderer.tesselateInWorldFixedTexture(
                        Blocks::stained_glass->defaultBlockState(), ringPositions[ring],
                        GlideRingGenerator::getIcon(ringSizes[ring]));
                    blockRenderer.clearLightOverride();
                }
                rendered = true;
            }
            for (int z = mDimension; z < mDimension + 16; ++z)
                for (int x = mX; x < mX + 16; ++x)
                    for (int y = mZ; y < mZ + 16; ++y)
                    {
                        unsigned int index = ((x - mX) << 11) | ((z - mDimension) << 7) |
                                             (y & 0x7f) | ((y & 0x80) << 8);
                        unsigned short element = cachedElements[index];
                        unsigned short extraElement = cachedExtraElements[index];
                        if (extraElement != 0)
                            hasOtherLayer = true;
                        if (pass == 2 && extraElement != 0)
                            element = extraElement;
                        if (element == 0xffff || elementToBlockId(element & 0x7fff) == 0)
                            continue;

                        BlockPos pos(x, y, z);
                        Block* block = Block::byId(elementToBlockId(element & 0x7fff));
                        if (pass == 0 && block->mBlockEntity)
                        {
                            boost::shared_ptr<BlockEntity> blockEntity = region.getBlockEntity(pos);
                            if (blockEntity && BlockEntityRenderDispatcher::instance->hasRenderer(blockEntity))
                                renderedBlockEntities.push_back(blockEntity);
                        }
                        int layer = block->getRenderLayer();
                        if (layer < pass)
                            continue;
                        if (layer > pass)
                        {
                            hasOtherLayer = true;
                            continue;
                        }
                        if (!started)
                        {
                            MemSect(0x1f);
                            GlStateManager::newList(listBase + pass, 0);
                            MemSect(0);
                            GlStateManager::pushMatrix();
                            GlStateManager::depthMask(true);
                            builder->useCompactVertices(true);
                            translateToPos();
                            builder->begin();
                            builder->offset((float)-mX, (float)-mZ, (float)-mDimension);
                            started = true;
                        }
                        const BlockState* state = pass == 2 && extraElement != 0
                            ? Block::getStateByElementId(element)
                            : region.getBlockState(pos);
                        if (blockRenderer.tesselateInWorld(state, pos, NULL, boost::shared_ptr<BlockEntity>()))
                            rendered = true;
                    }

            if (started)
            {
                builder->end();
                bounds.addBounds(builder->mBounds);
                GlStateManager::popMatrix();
                GlStateManager::endList(0);
                builder->useCompactVertices(false);
                builder->offset(0.0f, 0.0f, 0.0f);
            }
            if (rendered)
                levelRenderer->clearGlobalChunkFlag(mX, mZ, mDimension, mLevel, 4, pass);
            else
            {
                levelRenderer->setGlobalChunkFlag(mX, mZ, mDimension, mLevel, 4, pass);
                gRenderer->CBuffClear(listBase + pass);
            }
            if (pass == 0 && !hasOtherLayer)
            {
                levelRenderer->setGlobalChunkFlag(mX, mZ, mDimension, mLevel, 0x18, 0);
                gRenderer->CBuffClear(listBase + 1);
                gRenderer->CBuffClear(listBase + 2);
                break;
            }
        }
        if (mBoundingBox != NULL)
            mBoundingBox->set(bounds.mMinX, bounds.mMinY, bounds.mMinZ,
                              bounds.mMaxX, bounds.mMaxY, bounds.mMaxZ);
    }

    int key = globalIndex;
    EnterCriticalSection(mMutex);
    ChunkBlockEntityMap::iterator existing = mBlockEntities->find(key);
    if (existing != mBlockEntities->end())
        for (std::vector<boost::shared_ptr<BlockEntity> >::iterator it = existing->second.begin();
             it != existing->second.end(); ++it)
            (*it)->setRenderRemoveStage(1);
    for (std::vector<boost::shared_ptr<BlockEntity> >::iterator it = renderedBlockEntities.begin();
         it != renderedBlockEntities.end(); ++it)
    {
        std::vector<boost::shared_ptr<BlockEntity> >& entities = (*mBlockEntities)[key];
        std::vector<boost::shared_ptr<BlockEntity> >::iterator previous =
            std::find(entities.begin(), entities.end(), *it);
        if (previous == entities.end())
            entities.push_back(*it);
        else
            (*it)->setRenderRemoveStage(0);
    }
    LeaveCriticalSection(mMutex);
    if (!allEmpty || !ringPositions.empty())
        levelRenderer->setGlobalChunkFlag(mX, mZ, mDimension, mLevel, 1, 0);
}

double Chunk::distanceToSqr(boost::shared_ptr<Entity> entity) const
{
    double x = entity->mX - (double)mMaxX;
    double y = entity->mY - (double)mMaxZ;
    double z = entity->mZ - (double)mMaxDimension;
    return x * x + y * y + z * z;
}

void Chunk::_delete()
{
    reset();
    mLevel = NULL;
}

unsigned long long Chunk::cull(Culler* culler)
{
    mClipChunk->mVisible = culler->isVisible(mBoundingBox);
    return static_cast<unsigned long long>(reinterpret_cast<unsigned long>(this));
}

void Chunk::clearDirty()
{
    LevelRenderer* renderer = chunkLevelRenderer();
    if (renderer != NULL)
        renderer->clearGlobalChunkFlag(mX, mZ, mDimension, mLevel, 2, 0);
}
