#include "game/client/render/LevelRenderer.hpp"

#include <cmath>
#include <cstring>

#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Options.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/StringConversion.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/Block.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/core/Gui.hpp"
#include "game/item/RecordItem.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

static const unsigned int gChunkDimensionSizes[2] = { 80, 20 };

int gChunkDimensionBases[4];
int gEndInnerChunkX;
int gEndInnerChunkZ;
int gEndOuterChunkX;
int gEndOuterChunkZ;

LevelRenderer* gLevelRenderer;

static boost::unordered_map<int, int> gDisplayListLengths;

int getChunkIndex(int dimension, int x, int y, int z)
{
    if ((unsigned int)dimension < 2)
    {
        unsigned int size = gChunkDimensionSizes[dimension];
        int half = (int)size / 2;
        int offsetZ = half + z;
        int offsetX = half + x;

        if (offsetX >= 0 && offsetX < (int)size && (unsigned int)y < 16 && offsetZ >= 0 &&
            offsetZ < (int)size)
        {
            return gChunkDimensionBases[dimension] + (offsetX * size + offsetZ) * 16 + y;
        }

        return -1;
    }

    if (dimension != 2)
    {
        return -1;
    }

    unsigned int innerX = (x + 9) - gEndInnerChunkX;
    unsigned int innerZ = (z + 9) - gEndInnerChunkZ;

    if (innerX < 18 && (unsigned int)y < 16 && innerZ < 18)
    {
        return gChunkDimensionBases[2] + innerX * 0x120 + innerZ * 16 + y;
    }

    unsigned int outerX = (x + 6) - gEndOuterChunkX;
    unsigned int outerZ = (z + 6) - gEndOuterChunkZ;

    if (outerX < 12 && (unsigned int)y < 16 && outerZ < 12)
    {
        return gChunkDimensionBases[2] + outerX * 0xc0 + outerZ * 16 + y + 0x1440;
    }

    return -1;
}

LevelRenderer::DestroyedBlockManager::RecentBlock::RecentBlock(const BlockPos& blockPos, Level* sourceLevel)
    : pos(blockPos), level(sourceLevel), ticks(20), remove(false)
{
}

LevelRenderer::DestroyedBlockManager::RecentBlock::~RecentBlock()
{
    for (std::vector<AABB*>::iterator box = boxes.begin(); box != boxes.end(); ++box)
    {
        delete *box;
    }
}

LevelRenderer::DestroyedBlockManager::DestroyedBlockManager()
{
    InitializeCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::tick()
{
    EnterCriticalSection(&lock);

    for (unsigned int index = 0; index < blocks.size();)
    {
        if (--blocks[index]->ticks == 0)
        {
            delete blocks[index];
            blocks[index] = blocks[blocks.size() - 1];
            blocks.pop_back();
        }
        else
        {
            ++index;
        }
    }

    LeaveCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::updatedChunkAt(Level* sourceLevel, const BlockPos& chunkPos, int radius)
{
    EnterCriticalSection(&lock);

    for (unsigned int index = 0; index < blocks.size(); ++index)
    {
        RecentBlock* recent = blocks[index];

        if (recent->level == sourceLevel && recent->pos.x >= chunkPos.x &&
            recent->pos.x < chunkPos.x + 16 && recent->pos.y >= chunkPos.y &&
            recent->pos.y < chunkPos.y + 16 && recent->pos.z >= chunkPos.z &&
            recent->pos.z < chunkPos.z + 16)
        {
            recent->remove = true;
        }
    }

    if (radius < 2)
    {
        for (unsigned int index = 0; index < blocks.size();)
        {
            if (blocks[index]->remove)
            {
                delete blocks[index];
                blocks[index] = blocks[blocks.size() - 1];
                blocks.pop_back();
            }
            else
            {
                ++index;
            }
        }
    }

    LeaveCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::destroyingBlockAt(Level* sourceLevel, const BlockPos& blockPos)
{
    EnterCriticalSection(&lock);

    RecentBlock* recent = new RecentBlock(blockPos, sourceLevel);
    AABB* bounds = AABB::newTemp((double)blockPos.x, (double)blockPos.y, (double)blockPos.z,
                                 (double)(blockPos.x + 1), (double)(blockPos.y + 1),
                                 (double)(blockPos.z + 1));
    const BlockState* state = Minecraft::GetInstance()->mLevel->getBlockState(blockPos);
    Block* block = state->getBlock();

    if (block != 0)
    {
        boost::shared_ptr<Entity> none;
        block->addCollisionAABBs(state, sourceLevel, blockPos, bounds, &recent->boxes, none, false);
    }

    for (unsigned int index = 0; index < recent->boxes.size(); ++index)
    {
        AABB* box = recent->boxes[index];
        recent->boxes[index] = AABB::newPermanent(box->mMinX, box->mMinY, box->mMinZ, box->mMaxX,
                                                  box->mMaxY, box->mMaxZ);
    }

    blocks.push_back(recent);

    LeaveCriticalSection(&lock);
}

void LevelRenderer::DestroyedBlockManager::addAABBs(Level* sourceLevel, const AABB* area, std::vector<AABB*>* result)
{
    EnterCriticalSection(&lock);

    for (unsigned int index = 0; index < blocks.size(); ++index)
    {
        if (blocks[index]->level != sourceLevel)
        {
            continue;
        }

        for (unsigned int box = 0; box < blocks[index]->boxes.size(); ++box)
        {
            if (blocks[index]->boxes[box]->intersects(area->mMinX, area->mMinY, area->mMinZ,
                                                      area->mMaxX, area->mMaxY, area->mMaxZ))
            {
                AABB* hit = blocks[index]->boxes[box];
                result->push_back(AABB::newTemp(hit->mMinX, hit->mMinY, hit->mMinZ, hit->mMaxX,
                                                hit->mMaxY, hit->mMaxZ));
            }
        }
    }

    LeaveCriticalSection(&lock);
}

LevelRenderer::LevelRenderer(Minecraft* minecraft, Textures* textures)
    : mBlockEntities(11), mRenderableBlockEntities(11), mDestroyingBlocks(11)
{
    mEntityRenderDispatcher = Minecraft::GetInstance()->getEntityRenderDispatcher();

    for (int player = 0; player < 4; ++player)
    {
        mLevels[player] = 0;
        mClipChunks[player].data = 0;
        mClipChunks[player].length = 0;
        mActivePlayers[player] = 0;
        mBlockRenderers[player] = 0;
        mLastSortX[player] = -9999.0;
        mLastSortY[player] = -9999.0;
        mLastSortZ[player] = -9999.0;
    }

    mChunkGridWidth = 0;
    mChunkGridHeight = 0;
    mChunkGridDepth = 0;
    mDisplayList = 0;
    mRenderTick = 0;
    mStarList = 0;
    mSkyList = 0;
    mDarkList = 0;
    mMinChunkX = 0;
    mMinChunkY = 0;
    mMinChunkZ = 0;
    mMaxChunkX = 0;
    mMaxChunkY = 0;
    mMaxChunkZ = 0;
    mLastViewDistance = -1;
    mSkippedFrames = 2;
    mCountEntitiesTotal = 0;
    mCountEntitiesRendered = 0;
    mCountEntitiesHidden = 0;
    mField0x128 = 0;
    mField0x180 = 0;
    mField0x184 = MemoryTracker::genLists(1);

    for (int counter = 0; counter < 5; ++counter)
    {
        mRenderCounters[counter] = 0;
    }

    InitializeCriticalSection(&mChunkMutex);
    InitializeCriticalSection(&mBlockEntityMutex);
    InitializeCriticalSection(&mLevelMutex);
    InitializeCriticalSection(&mRenderableBlockEntityMutex);

    mTextures = textures;
    mCloudOffset = 0.0f;
    mHasDirtyChunks = false;
    mLastRebuildTime = 0;
    mMinecraft = minecraft;

    mDisplayList = MemoryTracker::genLists(getGlobalChunkCount() * 3);
    mGlobalChunkFlags = new unsigned char[getGlobalChunkCount()];
    memset(mGlobalChunkFlags, 0, getGlobalChunkCount());

    mStarList = MemoryTracker::genLists(4);

    GlStateManager::pushMatrix();
    GlStateManager::newList(mStarList, 0);
    renderStars();
    GlStateManager::endList(0);
    createCloudMesh();
    GlStateManager::popMatrix();

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    mSkyList = mStarList + 1;
    GlStateManager::newList(mSkyList, 0);
    GlStateManager::depthMask(false);

    for (int skyX = -384; skyX <= 384; skyX += 64)
    {
        for (int skyZ = -384; skyZ <= 384; skyZ += 64)
        {
            builder->begin();
            builder->vertex((float)skyX, 16.0f, (float)skyZ);
            builder->vertex((float)(skyX + 64), 16.0f, (float)skyZ);
            builder->vertex((float)(skyX + 64), 16.0f, (float)(skyZ + 64));
            builder->vertex((float)skyX, 16.0f, (float)(skyZ + 64));
            builder->end();
        }
    }

    GlStateManager::endList(0);

    mDarkList = mStarList + 2;
    GlStateManager::newList(mDarkList, 0);
    builder->begin();

    for (int darkX = -384; darkX <= 384; darkX += 64)
    {
        for (int darkZ = -384; darkZ <= 384; darkZ += 64)
        {
            builder->vertex((float)(darkX + 64), -16.0f, (float)darkZ);
            builder->vertex((float)darkX, -16.0f, (float)darkZ);
            builder->vertex((float)darkX, -16.0f, (float)(darkZ + 64));
            builder->vertex((float)(darkX + 64), -16.0f, (float)(darkZ + 64));
        }
    }

    builder->end();
    GlStateManager::endList(0);

    mHaloRingList = mStarList + 3;
    GlStateManager::newList(mHaloRingList, 0);
    builder->begin(1);
    builder->color(0xffffff);

    float ringU = 0.0f;

    for (int segment = 0; segment < 51; ++segment)
    {
        float distance = Mth::abs((float)segment - 25.0f);
        float over = distance < 19.0f ? 0.0f : distance - 19.0f;
        float halfHeight = (over * over) / 36.0f * 10.0f + 1.0f;
        float angle = (float)segment * 0.12566371f;

        builder->vertexUV(100.0f * cosf(angle) - 99.9f, 100.0f * sinf(angle), -halfHeight, ringU, 0.0f);
        builder->vertexUV(100.0f * cosf(angle) - 99.9f, 100.0f * sinf(angle), halfHeight, ringU, 1.0f);

        ringU -= 0.25;
    }

    builder->end();
    GlStateManager::endList(0);

    gLevelRenderer = this;
    mDestroyedBlocks = new DestroyedBlockManager();
}

LevelRenderer::~LevelRenderer()
{
}

unsigned int LevelRenderer::getGlobalChunkCount()
{
    return gChunkDimensionBases[3];
}

void LevelRenderer::renderStars()
{
    Random random(10842LL);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->begin();

    for (int star = 0; star < 1500; ++star)
    {
        double x = (double)(random.nextFloat() * 2.0f - 1.0f);
        double y = (double)(random.nextFloat() * 2.0f - 1.0f);
        double z = (double)(random.nextFloat() * 2.0f - 1.0f);
        double size = (double)(random.nextFloat() * 0.1f + 0.15f);
        double lengthSqr = z * z + x * x + y * y;

        if (lengthSqr < 1.0 && lengthSqr > 0.01)
        {
            double scale = 1.0 / sqrt(lengthSqr);

            x = x * scale;
            y = y * scale;
            z = z * scale;

            double centreX = x * 160.0;
            double centreY = y * 160.0;
            double centreZ = z * 160.0;
            double yaw = atan2(x, z);
            double sinYaw = sin(yaw);
            double cosYaw = cos(yaw);
            double pitch = atan2(sqrt(x * x + z * z), y);
            double sinPitch = sin(pitch);
            double cosPitch = cos(pitch);
            double roll = random.nextDouble() * 3.1415927f * 2.0;
            double sinRoll = sin(roll);
            double cosRoll = cos(roll);

            for (int corner = 0; corner < 4; ++corner)
            {
                double flat = 0.0;
                double cornerU = (double)((corner & 2) - 1) * size;
                double cornerV = (double)((corner + 1 & 2) - 1) * size;
                double rolledU = cornerU * cosRoll - cornerV * sinRoll;
                double rolledV = cornerV * cosRoll + cornerU * sinRoll;
                double pitchedY = rolledU * sinPitch + flat * cosPitch;
                double pitchedZ = flat * sinPitch - rolledU * cosPitch;
                double offsetX = pitchedZ * sinYaw - rolledV * cosYaw;
                double offsetZ = rolledV * sinYaw + pitchedZ * cosYaw;

                builder->vertex((float)(centreX + offsetX), (float)(centreY + pitchedY),
                                (float)(centreZ + offsetZ));
            }
        }
    }

    builder->end();
}

void LevelRenderer::createCloudMesh()
{
    mCloudLists = MemoryTracker::genLists(7);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    for (int face = 0; face < 7; ++face)
    {
        GlStateManager::newList(mCloudLists + face, 0);

        if (face == 0 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(0.7f, 0.7f, 0.7f, 0.8f);
                    builder->normal(0.0f, -1.0f, 0.0f);
                    builder->vertexUV((float)x, 0.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x, 0.0f, (float)z + 1.0f, u, v);
                }
            }

            builder->end();
        }

        if (face == 1 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(1.0f, 1.0f, 1.0f, 0.8f);
                    builder->normal(0.0f, 1.0f, 0.0f);
                    builder->vertexUV((float)x, 4.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z, u, v);
                    builder->vertexUV((float)x, 4.0f, (float)z, u, v);
                }
            }

            builder->end();
        }

        if (face == 2 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(0.9f, 0.9f, 0.9f, 0.8f);
                    builder->normal(-1.0f, 0.0f, 0.0f);
                    builder->vertexUV((float)x, 0.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x, 4.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x, 4.0f, (float)z, u, v);
                    builder->vertexUV((float)x, 0.0f, (float)z, u, v);
                }
            }

            builder->end();
        }

        if (face == 3 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(0.9f, 0.9f, 0.9f, 0.8f);
                    builder->normal(1.0f, 0.0f, 0.0f);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z + 1.0f, u, v);
                }
            }

            builder->end();
        }

        if (face == 4 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(0.8f, 0.8f, 0.8f, 0.8f);
                    builder->normal(-1.0f, 0.0f, 0.0f);
                    builder->vertexUV((float)x, 4.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z, u, v);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z, u, v);
                    builder->vertexUV((float)x, 0.0f, (float)z, u, v);
                }
            }

            builder->end();
        }

        if (face == 5 || face == 6)
        {
            builder->begin();

            for (int z = 0; z < 8; ++z)
            {
                for (int x = 0; x < 8; ++x)
                {
                    float u = ((float)x + 0.5f) * 0.00390625f;
                    float v = ((float)z + 0.5f) * 0.00390625f;

                    builder->color(0.8f, 0.8f, 0.8f, 0.8f);
                    builder->normal(1.0f, 0.0f, 0.0f);
                    builder->vertexUV((float)x, 0.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x + 1.0f, 0.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x + 1.0f, 4.0f, (float)z + 1.0f, u, v);
                    builder->vertexUV((float)x, 4.0f, (float)z + 1.0f, u, v);
                }
            }

            builder->end();
        }

        GlStateManager::endList(0);
    }
}

unsigned char LevelRenderer::activePlayers()
{
    unsigned char count = 0;

    for (int player = 0; player < 4; ++player)
    {
        if (mLevels[player] != 0)
        {
            ++count;
        }
    }

    return count;
}

void LevelRenderer::nonStackDirtyChunksAdded()
{
    mDirtyChunkStack.Push((int*)1);
}

void LevelRenderer::resortChunks(int x, int y, int z, int dimension)
{
    if (dimension == -1)
    {
        dimension = mMinecraft->mPlayer->mUserIndex;
    }

    EnterCriticalSection(&mChunkMutex);

    mMinChunkX = 0x7fffffff;
    mMinChunkY = 0x7fffffff;
    mMinChunkZ = 0x7fffffff;
    mMaxChunkX = (int)0x80000000;
    mMaxChunkY = (int)0x80000000;
    mMaxChunkZ = (int)0x80000000;

    int spanX = mChunkGridWidth * 16;

    for (int gridX = 0; gridX < mChunkGridWidth; ++gridX)
    {
        int worldX = gridX * 16;
        int wrapX = (worldX + spanX / 2) - (x - 8);

        if (wrapX < 0)
        {
            wrapX -= spanX - 1;
        }

        wrapX = worldX - (wrapX / spanX) * spanX;

        if (wrapX < mMinChunkX)
        {
            mMinChunkX = wrapX;
        }

        if (wrapX > mMaxChunkX)
        {
            mMaxChunkX = wrapX;
        }

        for (int gridZ = 0; gridZ < mChunkGridDepth; ++gridZ)
        {
            int worldZ = gridZ * 16;
            int wrapZ = (worldZ + spanX / 2) - (z - 8);

            if (wrapZ < 0)
            {
                wrapZ -= spanX - 1;
            }

            wrapZ = worldZ - (wrapZ / spanX) * spanX;

            if (wrapZ < mMinChunkZ)
            {
                mMinChunkZ = wrapZ;
            }

            if (wrapZ > mMaxChunkZ)
            {
                mMaxChunkZ = wrapZ;
            }

            for (int gridY = 0; gridY < mChunkGridHeight; ++gridY)
            {
                int worldY = gridY * 16;

                if (worldY < mMinChunkY)
                {
                    mMinChunkY = worldY;
                }

                if (worldY > mMaxChunkY)
                {
                    mMaxChunkY = worldY;
                }

                mClipChunks[dimension]
                    .data[(gridZ * mChunkGridHeight + gridY) * mChunkGridWidth + gridX]
                    .mChunk->setPos(wrapX, worldY, wrapZ);
            }
        }
    }

    nonStackDirtyChunksAdded();

    LeaveCriticalSection(&mChunkMutex);
}

void LevelRenderer::allChanged(int dimension)
{
    if (mLevels[dimension] == 0)
    {
        return;
    }

    Minecraft::GetInstance()->mGameRenderer->DisableUpdateThread(4);

    LeafBlock::mOldLeaves->setFancy(mMinecraft->mOptions->mFancyGraphics);
    LeafBlock::mNewLeaves->setFancy(mMinecraft->mOptions->mFancyGraphics);

    mLastViewDistance = mMinecraft->mOptions->mViewDistance;

    int viewDistance = (int)sqrtf(400.0f / (float)activePlayers());

    mMinecraft->mOptions->mViewDistance = viewDistance;
    mActivePlayers[dimension] = activePlayers();
    mChunkGridWidth = viewDistance;
    mChunkGridHeight = 16;
    mChunkGridDepth = viewDistance;

    if (mClipChunks[dimension].data != 0)
    {
        for (unsigned int index = 0; index < mClipChunks[dimension].length; ++index)
        {
            mClipChunks[dimension].data[index].mChunk->_delete();
            delete mClipChunks[dimension].data[index].mChunk;
        }

        delete[] mClipChunks[dimension].data;
    }

    mClipChunks[dimension] =
        arrayWithLength<ClipChunk>(mChunkGridWidth * mChunkGridHeight * mChunkGridDepth, true);

    for (unsigned int index = 0; index < mClipChunks[dimension].length; ++index)
    {
        mClipChunks[dimension].data[index].mChunk = 0;
    }

    mMinChunkX = 0;
    mMinChunkY = 0;
    mMinChunkZ = 0;
    mMaxChunkX = mChunkGridWidth;
    mMaxChunkY = mChunkGridHeight;
    mMaxChunkZ = mChunkGridDepth;

    int globalIndex = 0;

    for (int gridX = 0; gridX < mChunkGridWidth; ++gridX)
    {
        for (int gridY = 0; gridY < mChunkGridHeight; ++gridY)
        {
            for (int gridZ = 0; gridZ < mChunkGridDepth; ++gridZ)
            {
                ClipChunk* clipChunk =
                    &mClipChunks[dimension]
                         .data[(gridZ * mChunkGridHeight + gridY) * mChunkGridWidth + gridX];

                clipChunk->mChunk = new Chunk(mLevels[dimension], mBlockEntities, mBlockEntityMutex,
                                              gridX * 16, gridY * 16, gridZ * 16, clipChunk);
                clipChunk->mVisible = true;
                clipChunk->mChunk->mUnknown = globalIndex;
                ++globalIndex;
            }
        }
    }

    nonStackDirtyChunksAdded();

    EnterCriticalSection(&mRenderableBlockEntityMutex);
    mRenderableBlockEntities.clear();
    LeaveCriticalSection(&mRenderableBlockEntityMutex);

    if (mLevels[dimension] != 0)
    {
        boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();

        if (camera)
        {
            resortChunks((int)Mth::lfloor(camera->mX), (int)Mth::lfloor(camera->mY),
                         (int)Mth::lfloor(camera->mZ), dimension);
        }
    }

    mSkippedFrames = 2;

    Minecraft::GetInstance()->mGameRenderer->EnableUpdateThread(4);
}

void LevelRenderer::tick()
{
    ++mRenderTick;

    if (mRenderTick % 20 != 0)
    {
        return;
    }

    boost::unordered_map<int, BlockDestructionProgress*>::iterator progress = mDestroyingBlocks.begin();

    while (progress != mDestroyingBlocks.end())
    {
        if (mRenderTick - progress->second->getUpdatedRenderTick() > 400)
        {
            delete progress->second;
            progress = mDestroyingBlocks.erase(progress);
        }
        else
        {
            ++progress;
        }
    }
}

void LevelRenderer::clear()
{
    boost::unordered_map<int, int>::iterator entry = gDisplayListLengths.find(mDisplayList);

    if (entry != gDisplayListLengths.end())
    {
        GlStateManager::deleteLists(mDisplayList, entry->second);
        gDisplayListLengths.erase(entry);
    }
}

void LevelRenderer::allChanged()
{
    allChanged(mMinecraft->mPlayer->mUserIndex);
}

bool LevelRenderer::sortEntDisFunc(std::pair<int, double> first, std::pair<int, double> second)
{
    return first.second < second.second;
}

int LevelRenderer::getDimensionIndexFromId(int id)
{
    return (3 - id) % 3;
}

bool LevelRenderer::isGlobalIndexInSameDimension(int index, Level* level)
{
    int dimensionIndex = getDimensionIndexFromId(level->mDimension->getType()->getId());

    if (index < gChunkDimensionBases[1])
    {
        return dimensionIndex == 0;
    }

    if (index < gChunkDimensionBases[2])
    {
        return dimensionIndex == 1;
    }

    return dimensionIndex == (index < gChunkDimensionBases[3] ? 2 : -1);
}

void LevelRenderer::setupDestroyState()
{
    GlStateManager::blendFuncSeparate(8, 2, 1, 0);
    GlStateManager::blendFunc(8, 2);
    GlStateManager::enableBlend();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 0.5f);
    GlStateManager::polygonOffset(-3.0f, -3.0f);
    GlStateManager::enablePolygonOffset();
    GlStateManager::alphaFunc(4, 0.01f);
    GlStateManager::enableAlphaTest();
    GlStateManager::pushMatrix();
}

void LevelRenderer::restoreDestroyState()
{
    GlStateManager::disableAlphaTest();
    GlStateManager::polygonOffset(0.0f, 0.0f);
    GlStateManager::disablePolygonOffset();
    GlStateManager::enableAlphaTest();
    GlStateManager::depthMask(true);
    GlStateManager::popMatrix();
}

unsigned int LevelRenderer::getBucketMask(double x, double y, double z, ClipChunk* chunk)
{
    unsigned int mask = 1;

    if (x <= (double)chunk->mMaxX)
    {
        mask = 0x21;
    }

    if ((double)chunk->mMinX <= x)
    {
        mask |= 8;
    }

    if ((double)chunk->mMinZ <= z)
    {
        mask |= 0x10;
    }

    if (z <= (double)chunk->mMaxZ)
    {
        mask |= 4;
    }

    if (y <= (double)chunk->mMaxY)
    {
        mask |= 0x40;
    }

    if ((double)chunk->mMinY <= y)
    {
        mask |= 2;
    }

    return mask;
}

void LevelRenderer::setupCameraInsideAreas(boost::shared_ptr<Entity> camera)
{
    if (!Minecraft::InMiniGame((EMiniGameId)3, true))
    {
        return;
    }

    mCameraInsideAreas.clear();

    std::vector<AABB*> areas;
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();

    if (ruleset != 0)
    {
        ruleset->getActiveViewAreas(&areas);

        Vec3* position = Vec3::newTemp(camera->mX * 0.0625, camera->mY * 0.0625, camera->mZ * 0.0625);

        for (unsigned int index = 0; index < areas.size(); ++index)
        {
            if (areas[index]->insideXZ(position))
            {
                mCameraInsideAreas.push_back(areas[index]);
            }
        }
    }
}

unsigned int LevelRenderer::clippedByCameraInsideAreas(ClipChunk* chunk)
{
    if (!Minecraft::InMiniGame((EMiniGameId)3, true))
    {
        return 0;
    }

    if (mCameraInsideAreas.empty())
    {
        return 0;
    }

    unsigned int clipped = 1;
    Vec3* position = Vec3::newTemp((double)Mth::intFloorDiv(chunk->mX, 16),
                                   (double)Mth::intFloorDiv(chunk->mY, 16),
                                   (double)Mth::intFloorDiv(chunk->mZ, 16));

    for (unsigned int index = 0; index < mCameraInsideAreas.size(); ++index)
    {
        if (mCameraInsideAreas[index]->insideXZ(position))
        {
            clipped = 0;
        }
    }

    return clipped;
}

void LevelRenderer::render(boost::shared_ptr<Entity> camera, int layer, double partialTick, bool sort)
{
    int dimension = mMinecraft->mPlayer->mUserIndex;

    if (mActivePlayers[dimension] != activePlayers() ||
        mMinecraft->mOptions->mViewDistance != mLastViewDistance)
    {
        allChanged();
    }

    if (layer == 0)
    {
        for (int counter = 0; counter < 5; ++counter)
        {
            mRenderCounters[counter] = 0;
        }
    }

    double deltaX = camera->mX - mLastSortX[dimension];
    double deltaY = camera->mY - mLastSortY[dimension];
    double deltaZ = camera->mZ - mLastSortZ[dimension];

    if (deltaZ * deltaZ + deltaX * deltaX + deltaY * deltaY > 16.0)
    {
        mLastSortX[dimension] = camera->mX;
        mLastSortY[dimension] = camera->mY;
        mLastSortZ[dimension] = camera->mZ;

        resortChunks((int)Mth::lfloor(camera->mX), (int)Mth::lfloor(camera->mY),
                     (int)Mth::lfloor(camera->mZ), -1);
    }

    Lighting::turnOff();

    renderChunks(0, mClipChunks[dimension].length, layer, partialTick);
}

void LevelRenderer::renderHaloRing(float partialTick)
{
    if (!mMinecraft->mLevel->mDimension->isNaturalDimension())
    {
        return;
    }

    GlStateManager::disableAlphaTest();
    GlStateManager::disableTexture();
    GlStateManager::depthMask(false);
    GlStateManager::enableFog();

    int dimension = mMinecraft->mPlayer->mUserIndex;
    Vec3* skyColor = mLevels[dimension]->getSkyColor(mMinecraft->getCameraEntity(), partialTick);
    float brightness = ((float)skyColor->x + (float)skyColor->x + (float)skyColor->z +
                        (float)skyColor->y + (float)skyColor->y + (float)skyColor->y) /
                           6.0f * 0.4f +
                       0.6f;

    GlStateManager::color3f(brightness, brightness, brightness);
    GlStateManager::fogMode(1);

    float fogStart = GlStateManager::getFogStart();
    float fogEnd = GlStateManager::getFogEnd();

    GlStateManager::fogStart(100.0f);
    GlStateManager::fogEnd(20.0f);

    Lighting::turnOn();
    GlStateManager::depthMask(false);

    mTextures->bindTexture(L"misc/haloRing");

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    bool mipmap = builder->setMipmapEnable(true);

    GlStateManager::pushMatrix();
    GlStateManager::rotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(90.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::callList(mHaloRingList);
    GlStateManager::popMatrix();

    builder->setMipmapEnable(mipmap);

    GlStateManager::depthMask(true);
    GlStateManager::enableTexture();
    GlStateManager::enableAlphaTest();
    GlStateManager::disableFog();
    GlStateManager::fogStart(fogStart);
    GlStateManager::fogEnd(fogEnd);
}

bool LevelRenderer::isInCloud(double x, double y, double z, float partialTick)
{
    return false;
}

void LevelRenderer::setGlobalChunkFlag(int index, unsigned char mask, unsigned char shift)
{
    if (index == -1)
    {
        return;
    }

    mGlobalChunkFlags[index] |= (unsigned char)(mask << shift);
}

int LevelRenderer::getGlobalIndexForChunk(int x, int y, int z, Level* level)
{
    return getChunkIndex(getDimensionIndexFromId(level->mDimension->getType()->getId()), x >> 4,
                         y >> 4, z >> 4);
}

bool LevelRenderer::getGlobalChunkFlag(int x, int y, int z, Level* level, unsigned char mask, unsigned char shift)
{
    unsigned char bits = (unsigned char)(mask << shift);
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index == -1)
    {
        return false;
    }

    return (bits & mGlobalChunkFlags[index]) == bits;
}

void LevelRenderer::renderLineBox(const AABB* box, int color)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->begin(5);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->end();

    builder->begin(5);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->end();

    builder->begin(4);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->end();
}

void LevelRenderer::renderColoredLineBox(const AABB* box, int color, bool splitLargeFaces)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->begin(5);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->end();

    builder->begin(5);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->end();

    builder->begin(4);
    builder->color(color);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->end();

    if (!splitLargeFaces)
    {
        return;
    }

    if (Mth::abs(box->mMaxX) - Mth::abs(box->mMinX) > 1.5)
    {
        double midX = (box->mMaxX - box->mMinX) * 0.5 + box->mMinX;

        builder->begin(5);
        builder->color(color);
        builder->vertex((float)midX, (float)box->mMinY, (float)box->mMinZ);
        builder->vertex((float)midX, (float)box->mMinY, (float)box->mMaxZ);
        builder->vertex((float)midX, (float)box->mMaxY, (float)box->mMaxZ);
        builder->vertex((float)midX, (float)box->mMaxY, (float)box->mMinZ);
        builder->vertex((float)midX, (float)box->mMinY, (float)box->mMinZ);
        builder->end();
    }

    if (Mth::abs(box->mMaxZ) - Mth::abs(box->mMinZ) > 1.5)
    {
        double midZ = (box->mMaxZ - box->mMinZ) * 0.5 + box->mMinZ;

        builder->begin(5);
        builder->color(color);
        builder->vertex((float)box->mMinX, (float)box->mMinY, (float)midZ);
        builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)midZ);
        builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)midZ);
        builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)midZ);
        builder->vertex((float)box->mMinX, (float)box->mMinY, (float)midZ);
        builder->end();
    }
}

void LevelRenderer::renderLine(std::vector<GameRenderer::PositionStruct>* positions, std::vector<int>* colors, std::vector<int>* alphas, float width)
{
    float halfWidth = width * 0.5f;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    GlStateManager::disableCull();
    GlStateManager::disableTexture();
    GlStateManager::lineWidth(2.0f);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::enableBlend();
    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
    GlStateManager::depthMask(false);
    GlStateManager::disableAlphaTest();

    builder->begin(1);

    for (unsigned int index = 0; index < positions->size(); ++index)
    {
        GameRenderer::PositionStruct position = positions->at(index);
        int color = colors->at(index);
        int alpha = alphas->at(index);
        float deltaZ;
        float deltaX;

        if (index == 0)
        {
            deltaZ = 0.0f;
            deltaX = 0.0f;

            if (!positions->empty())
            {
                deltaZ = position.z - positions->at(1).z;
                deltaX = position.x - positions->at(1).x;
            }
        }
        else
        {
            deltaZ = positions->at(index - 1).z - position.z;
            deltaX = positions->at(index - 1).x - position.x;
        }

        float angle = atan2f(deltaZ, deltaX) + 1.5707964f;

        builder->color(color, alpha);
        builder->vertex(halfWidth * cosf(angle) + position.x, position.y,
                        halfWidth * sinf(angle) + position.z);
        builder->vertex(halfWidth * cosf(angle + 3.1415927f) + position.x, position.y,
                        position.z + halfWidth * sinf(angle + 3.1415927f));
    }

    builder->end();

    GlStateManager::enableCull();
    GlStateManager::enableAlphaTest();
    GlStateManager::depthMask(true);
    GlStateManager::enableTexture();
    GlStateManager::disableBlend();
}

void LevelRenderer::renderColoredBox(const AABB* box, int color, int alpha)
{
    GlStateManager::disableCull();

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->begin(3);
    builder->color(color, alpha);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->end();

    GlStateManager::enableCull();
}

void LevelRenderer::renderStringInWorld(boost::shared_ptr<Entity> entity, const std::wstring& text, Vec3* position)
{
    float xRot = entity->mXRot;
    float yRot = entity->mYRot;
    Font* font = BlockEntityRenderDispatcher::instance->getFont();

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)position->x, (float)position->y, (float)position->z);
    GlStateManager::normal3f(0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(-yRot, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(xRot, 1.0f, 0.0f, 0.0f);
    GlStateManager::scalef(-0.025f, -0.025f, 0.025f);
    GlStateManager::depthMask(false);

    int halfWidth = font->width(text) / 2;

    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    GlStateManager::disableTexture();
    builder->begin();
    builder->color(0.0f, 0.0f, 0.0f, 0.25f);
    builder->vertex((float)(-(halfWidth + 1)), -1.0f, 0.0f);
    builder->vertex((float)(-(halfWidth + 1)), 9.0f, 0.0f);
    builder->vertex((float)(halfWidth + 1), 9.0f, 0.0f);
    builder->vertex((float)(halfWidth + 1), -1.0f, 0.0f);
    builder->end();
    GlStateManager::enableTexture();
    GlStateManager::depthMask(true);

    font->draw(text, -halfWidth, 0, 0xffffffff);

    GlStateManager::disableBlend();
    GlStateManager::popMatrix();
}

void LevelRenderer::renderStringOnPlane(boost::shared_ptr<Entity> entity, const std::wstring& text, Vec3* position, Vec3* axis)
{
    Font* font = BlockEntityRenderDispatcher::instance->getFont();

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)position->x, (float)position->y, (float)position->z);
    GlStateManager::normal3f(0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(1.5707964f * 57.295776f, (float)axis->x, (float)axis->y, (float)axis->z);
    GlStateManager::scalef(-1.0f, -1.0f, 1.0f);
    GlStateManager::depthMask(false);

    int halfWidth = font->width(text) / 2;

    GlStateManager::enableBlend();
    GlStateManager::disableCull();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::depthMask(true);

    font->draw(text, -halfWidth, 0, 0xffffffff);

    GlStateManager::disableBlend();
    GlStateManager::enableCull();
    GlStateManager::popMatrix();
}

void LevelRenderer::renderDebugVolume(boost::shared_ptr<Entity> entity, const std::wstring& text, const AABB* box, int color, bool splitLargeFaces, bool depthTest, bool thickLines, bool raiseText)
{
    if (!depthTest)
    {
        GlStateManager::disableDepthTest();
    }

    GlStateManager::disableLighting();

    Vec3* centre = box->getCenter();

    if (raiseText)
    {
        centre->move(0.0, 1.0, 0.0);
    }

    renderStringInWorld(entity, text, centre);

    GlStateManager::disableTexture();

    int alpha = 0x3f;
    float width = 2.0f;

    if (thickLines)
    {
        alpha = 0x7f;
        width = 4.0f;
    }

    GlStateManager::lineWidth(width);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::enableBlend();
    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
    GlStateManager::depthMask(false);

    renderColoredLineBox(box, color, splitLargeFaces);
    renderColoredBox(box, color, alpha);

    GlStateManager::depthMask(true);
    GlStateManager::enableTexture();
    GlStateManager::disableBlend();

    if (!depthTest)
    {
        GlStateManager::enableDepthTest();
    }
}

void LevelRenderer::renderFancyTextDebugVolume(boost::shared_ptr<Entity> entity, const std::wstring& text, const AABB* box, int color, bool splitLargeFaces, bool depthTest, bool thickLines)
{
    if (!depthTest)
    {
        GlStateManager::disableDepthTest();
    }

    GlStateManager::disableLighting();

    Vec3* centre = box->getCenter();

    renderStringOnPlane(entity, text + L" TOP", Vec3::newTemp(centre->x, box->mMaxY, centre->z),
                        Vec3::newTemp(1.0, 0.0, 0.0));
    renderStringOnPlane(entity, text + L" BOTTOM", Vec3::newTemp(centre->x, box->mMinY, centre->z),
                        Vec3::newTemp(1.0, 0.0, 0.0));
    renderStringOnPlane(entity, text + L" NORTH", Vec3::newTemp(centre->x, centre->y, box->mMinZ),
                        Vec3::newTemp(0.0, 0.0, 0.0));
    renderStringOnPlane(entity, text + L" SOUTH", Vec3::newTemp(centre->x, centre->y, box->mMaxZ),
                        Vec3::newTemp(0.0, 0.0, 0.0));
    renderStringOnPlane(entity, text + L" EAST", Vec3::newTemp(box->mMaxX, centre->y, centre->z),
                        Vec3::newTemp(0.0, 1.0, 0.0));
    renderStringOnPlane(entity, text + L" WEST", Vec3::newTemp(box->mMinX, centre->y, centre->z),
                        Vec3::newTemp(0.0, -1.0, 0.0));

    GlStateManager::disableTexture();

    int alpha = 0x3f;
    float width = 2.0f;

    if (thickLines)
    {
        alpha = 0x7f;
        width = 4.0f;
    }

    GlStateManager::lineWidth(width);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::enableBlend();
    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
    GlStateManager::depthMask(false);

    renderColoredLineBox(box, color, splitLargeFaces);
    renderColoredBox(box, color, alpha);

    GlStateManager::depthMask(true);
    GlStateManager::enableTexture();
    GlStateManager::disableBlend();

    if (!depthTest)
    {
        GlStateManager::enableDepthTest();
    }
}

void LevelRenderer::renderDebugWireframeVolume(boost::shared_ptr<Entity> entity, const std::wstring& text, const AABB* box, int color, bool splitLargeFaces, bool depthTest, bool thickLines, bool raiseText)
{
    if (!depthTest)
    {
        GlStateManager::disableDepthTest();
    }

    GlStateManager::disableLighting();

    Vec3* centre = box->getCenter();

    if (raiseText)
    {
        centre->move(0.0, 1.0, 0.0);
    }

    renderStringInWorld(entity, text, centre);

    GlStateManager::disableTexture();

    if (thickLines)
    {
        GlStateManager::lineWidth(4.0f);
    }
    else
    {
        GlStateManager::lineWidth(2.0f);
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::enableBlend();
    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
    GlStateManager::depthMask(false);

    renderColoredLineBox(box, color, splitLargeFaces);

    GlStateManager::depthMask(true);
    GlStateManager::enableTexture();
    GlStateManager::disableBlend();

    if (!depthTest)
    {
        GlStateManager::enableDepthTest();
    }
}

void LevelRenderer::setDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, Level* level)
{
    if (level == 0)
    {
        level = mLevels[mMinecraft->mPlayer->mUserIndex];
    }

    int chunkMinX = Mth::intFloorDiv(minX, 16);
    int chunkMinY = Mth::intFloorDiv(minY, 16);
    int chunkMinZ = Mth::intFloorDiv(minZ, 16);
    int chunkMaxX = Mth::intFloorDiv(maxX, 16);
    int chunkMaxY = Mth::intFloorDiv(maxY, 16);
    int chunkMaxZ = Mth::intFloorDiv(maxZ, 16);

    for (int chunkX = chunkMinX; chunkX <= chunkMaxX; ++chunkX)
    {
        for (int chunkY = chunkMinY; chunkY <= chunkMaxY; ++chunkY)
        {
            for (int chunkZ = chunkMinZ; chunkZ <= chunkMaxZ; ++chunkZ)
            {
                int index = getGlobalIndexForChunk(chunkX * 16, chunkY * 16, chunkZ * 16, level);

                if (index >= 0)
                {
                    mDirtyChunkStack.Push((int*)(index + 2));
                    PIXSetMarkerDeprecated(0.0f, "Setting chunk %d %d %d dirty", chunkX * 16,
                                           chunkY * 16, chunkZ * 16);
                }
            }
        }
    }
}

void LevelRenderer::blockChanged(Level* level, const BlockPos& pos, const BlockState* oldState, const BlockState* newState, int flags, bool immediate)
{
    setDirty(pos.x - 1, pos.y - 1, pos.z - 1, pos.x + 1, pos.y + 1, pos.z + 1, 0);
}

void LevelRenderer::blockLightChanged(const BlockPos& pos)
{
    setDirty(pos.x - 1, pos.y - 1, pos.z - 1, pos.x + 1, pos.y + 1, pos.z + 1, 0);
}

void LevelRenderer::setBlocksDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, Level* level)
{
    setDirty(minX - 1, minY - 1, minZ - 1, maxX + 1, maxY + 1, maxZ + 1, level);
}

void LevelRenderer::playSound(boost::shared_ptr<Player> player, const SoundEvent* sound, SoundSource::ESoundSource source, double x, double y, double z, float volume, float pitch, float range)
{
}

void LevelRenderer::addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params)
{
    addParticle(level, type, overrideLimiter, false, x, y, z, xa, ya, za, params);
}

int LevelRenderer::calculateParticleLevel(Level* level, bool overrideLimiter)
{
    int particles = mMinecraft->mOptions->mParticles;

    if (overrideLimiter && particles == 2)
    {
        if (level->mRandom->nextInt(10) != 0)
        {
            return 2;
        }
    }
    else if (particles != 1)
    {
        return particles;
    }

    return level->mRandom->nextInt(3) == 0 ? 2 : 1;
}

void LevelRenderer::addParticleInternal(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params)
{
    addParticleInternal(level, type, overrideLimiter, false, x, y, z, xa, ya, za, params);
}

void LevelRenderer::addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, bool force, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params)
{
    addParticleInternal(level, type, overrideLimiter, force, x, y, z, xa, ya, za, params);
}

void LevelRenderer::addParticle(Level* level, const ParticleType* particle, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params)
{
    addParticle(level, particle->getId(), particle->getOverrideLimiter(), x, y, z, xa, ya, za, params);
}

void LevelRenderer::destroyBlockProgress(int breakerId, const BlockPos& pos, int progress)
{
    boost::unordered_map<int, BlockDestructionProgress*>::iterator entry =
        mDestroyingBlocks.find(breakerId);

    if ((unsigned int)progress >= 10)
    {
        if (entry != mDestroyingBlocks.end())
        {
            delete entry->second;
            mDestroyingBlocks.erase(entry);
        }

        return;
    }

    BlockDestructionProgress* destruction = 0;

    if (entry != mDestroyingBlocks.end())
    {
        destruction = entry->second;
    }

    if (destruction == 0 || destruction->getPos().x != pos.x || destruction->getPos().y != pos.y ||
        destruction->getPos().z != pos.z)
    {
        destruction = new BlockDestructionProgress(breakerId, pos);
        mDestroyingBlocks[breakerId] = destruction;
    }

    destruction->setProgress(progress);
    destruction->updateTick(mRenderTick);
}

void LevelRenderer::registerTextures(IconRegister* iconRegister)
{
    for (int stage = 0; stage < 10; ++stage)
    {
        mDestroyStages[stage] = iconRegister->registerIcon(std::wstring(L"destroy_") + _toString(stage));
    }
}

void LevelRenderer::fullyFlagRenderableBlockEntitiesToBeRemoved()
{
    EnterCriticalSection(&mBlockEntityMutex);

    for (ChunkBlockEntityMap::iterator entry = mBlockEntities.begin(); entry != mBlockEntities.end();
         ++entry)
    {
        std::vector<boost::shared_ptr<BlockEntity> >& list = entry->second;

        for (std::vector<boost::shared_ptr<BlockEntity> >::iterator blockEntity = list.begin();
             blockEntity != list.end(); ++blockEntity)
        {
            (*blockEntity)->upgradeRenderRemoveStage();
        }
    }

    LeaveCriticalSection(&mBlockEntityMutex);
}

int LevelRenderer::getGlobalIndexForChunk(int x, int y, int z, int dimension)
{
    return getChunkIndex(getDimensionIndexFromId(dimension), x >> 4, y >> 4, z >> 4);
}

void LevelRenderer::setGlobalChunkFlag(int x, int y, int z, Level* level, unsigned char mask, unsigned char shift)
{
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index != -1)
    {
        mGlobalChunkFlags[index] |= (unsigned char)(mask << shift);
    }
}

void LevelRenderer::clearGlobalChunkFlag(int x, int y, int z, Level* level, unsigned char mask, unsigned char shift)
{
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index != -1)
    {
        mGlobalChunkFlags[index] &= (unsigned char)~(mask << shift);
    }
}

unsigned char LevelRenderer::getGlobalChunkFlags(int x, int y, int z, Level* level)
{
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index == -1)
    {
        return 0;
    }

    return mGlobalChunkFlags[index];
}

void LevelRenderer::setGlobalChunkFlags(int x, int y, int z, Level* level, unsigned char flags)
{
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index != -1)
    {
        mGlobalChunkFlags[index] = flags;
    }
}

unsigned char LevelRenderer::incGlobalChunkRefCount(int x, int y, int z, Level* level)
{
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index == -1)
    {
        return 0;
    }

    unsigned char flags = mGlobalChunkFlags[index];
    unsigned char count = (unsigned char)((flags >> 5) + 1);

    mGlobalChunkFlags[index] = (unsigned char)(count * 32 | (flags & 0x1f));

    return count;
}

unsigned char LevelRenderer::decGlobalChunkRefCount(int x, int y, int z, Level* level)
{
    int index = getGlobalIndexForChunk(x, y, z, level);

    if (index == -1)
    {
        return 0;
    }

    unsigned char flags = mGlobalChunkFlags[index];
    unsigned char count = (unsigned char)((flags >> 5) - 1);

    mGlobalChunkFlags[index] = (unsigned char)(count * 32 | (flags & 0x1f));

    return count;
}

void LevelRenderer::playerRemoved(boost::shared_ptr<Entity> player)
{
}

void LevelRenderer::playStreamingMusic(const SoundEvent* sound, const BlockPos& pos, bool playing)
{
    if (sound != NULL)
    {
        RecordItem* record = RecordItem::getBySound(sound);
        if (record != NULL)
        {
            std::wstring name = record->getDisplayName();
            if (mMinecraft != NULL && mMinecraft->mGui != NULL)
            {
                mMinecraft->mGui->setNowPlaying(name);
            }
        }
    }
}

void LevelRenderer::addParticleInternal(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, bool force, double x, double y, double z, double xa, double ya, double za, arrayWithLength<int> params)
{
    if (mMinecraft == NULL || level == NULL) return;
    boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
    if (!camera || mMinecraft->mParticleEngine == NULL) return;
    int particleSetting = calculateParticleLevel(level, overrideLimiter);
    if (particleSetting > 1) return;
    double maxDist = force ? (1024.0 * 1024.0) : (32.0 * 32.0);
    double dx = camera->mX - x;
    double dy = camera->mY - y;
    double dz = camera->mZ - z;
    if (dx * dx + dy * dy + dz * dz > maxDist) return;
    mMinecraft->mParticleEngine->createParticle(type, x, y, z, xa, ya, za, params, level);
}

void LevelRenderer::entityAdded(boost::shared_ptr<Entity> entity)
{
    if (!entity) return;
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    if (player)
    {
        player->prepareCustomTextures();
    }
}

void LevelRenderer::entityRemoved(boost::shared_ptr<Entity> entity)
{
}

void LevelRenderer::globalLevelEvent(int type, const BlockPos& pos, int data)
{
    levelEvent(boost::shared_ptr<Player>(), type, pos, data);
}

void LevelRenderer::levelEvent(boost::shared_ptr<Player> player, int type, const BlockPos& pos, int data)
{
    if (mMinecraft == NULL) return;
    Level* level = mMinecraft->mLevel;
    if (level == NULL) return;
    Random* random = level->mRandom;

    switch (type)
    {
    case 2000:
    {
        int dir = data % 6;
        double xa = 0.0, ya = 0.0, za = 0.0;
        if (dir == 0) ya = -0.1;
        else if (dir == 1) ya = 0.1;
        else if (dir == 2) za = -0.1;
        else if (dir == 3) za = 0.1;
        else if (dir == 4) xa = -0.1;
        else if (dir == 5) xa = 0.1;
        for (int i = 0; i < 10; ++i)
        {
            double px = pos.x + 0.5 + (random ? (random->nextDouble() - 0.5) * 0.2 : 0.0);
            double py = pos.y + 0.5 + (random ? (random->nextDouble() - 0.5) * 0.2 : 0.0);
            double pz = pos.z + 0.5 + (random ? (random->nextDouble() - 0.5) * 0.2 : 0.0);
            addParticle(level, ePARTICLE_SMOKE, false, px, py, pz, xa, ya, za, arrayWithLength<int>());
        }
        break;
    }
    case 2001:
    {
        Block* block = Block::byId(data & 0xFFF);
        if (block != NULL)
        {
            for (int i = 0; i < 10; ++i)
            {
                double px = pos.x + (random ? random->nextDouble() : 0.5);
                double py = pos.y + (random ? random->nextDouble() : 0.5);
                double pz = pos.z + (random ? random->nextDouble() : 0.5);
                double xa = (px - (pos.x + 0.5)) * 4.0;
                double ya = (py - (pos.y + 0.5)) * 4.0;
                double za = (pz - (pos.z + 0.5)) * 4.0;
                arrayWithLength<int> p(1, false);
                p.data[0] = data;
                addParticle(level, ePARTICLE_BLOCK_CRACK, false, px, py, pz, xa, ya, za, p);
            }
        }
        break;
    }
    default:
        break;
    }
}

