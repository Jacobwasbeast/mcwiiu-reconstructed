#include "game/client/render/LevelRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Frustum.hpp"
#include "game/core/FrustumCuller.hpp"
#include "game/core/FrustumData.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/InfoBarManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Options.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/DyeItem.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/System.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/Gui.hpp"
#include "game/item/RecordItem.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/CauldronBlock.hpp"
#include "game/world/block/DragonEggBlock.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "platform/wiiu/CProfile.hpp"

static const unsigned int gChunkDimensionSizes[2] = { 80, 20 };

int gChunkDimensionBases[4];
int gEndInnerChunkX;
int gEndInnerChunkZ;
int gEndOuterChunkX;
int gEndOuterChunkZ;

LevelRenderer* gLevelRenderer;

static boost::unordered_map<int, int> gDisplayListLengths;

extern void instructionSynchronize();
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
    mAuxiliaryDisplayList = MemoryTracker::genLists(1);
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

void LevelRenderer::setLevel(int dimension, MultiPlayerLevel* level)
{
    if (dimension < 0 || dimension >= 4)
        return;

    EnterCriticalSection(&mLevelMutex);

    Level* oldLevel = mLevels[dimension];
    if (oldLevel != 0)
    {
        int oldReferences = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (mLevels[i] == oldLevel)
                ++oldReferences;
        }
        if (oldReferences == 1)
            oldLevel->removeListener(this);
    }

    mLevels[dimension] = level;
    mLastSortX[dimension] = -9999.0;
    mLastSortY[dimension] = -9999.0;
    mLastSortZ[dimension] = -9999.0;

    if (mBlockRenderers[dimension] != 0)
    {
        delete mBlockRenderers[dimension];
        mBlockRenderers[dimension] = 0;
    }

    if (level != 0)
        mBlockRenderers[dimension] = new BlockRenderer(level);

    if (level == 0)
    {
        if (mClipChunks[dimension].data != 0)
        {
            for (unsigned int index = 0; index < mClipChunks[dimension].length; ++index)
            {
                mClipChunks[dimension].data[index].mChunk->_delete();
                delete mClipChunks[dimension].data[index].mChunk;
            }
            delete[] mClipChunks[dimension].data;
            mClipChunks[dimension].data = 0;
            mClipChunks[dimension].length = 0;
        }

        if (mMinecraft != 0 && mMinecraft->mLevel != 0
            && dimension == getDimensionIndexFromId(mMinecraft->mLevel->mDimension->getType()->getId()))
        {
            EnterCriticalSection(&mBlockEntityMutex);
            mBlockEntities.clear();
            LeaveCriticalSection(&mBlockEntityMutex);
        }

        LeaveCriticalSection(&mLevelMutex);
        return;
    }

    int newReferences = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (mLevels[i] == level)
            ++newReferences;
    }
    if (newReferences == 1)
        level->addListener(this);

    LeaveCriticalSection(&mLevelMutex);
    allChanged(dimension);
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

bool LevelRenderer::shouldEntityGlow(boost::shared_ptr<Entity> entity, boost::shared_ptr<Entity> camera, Culler* culler)
{
    bool cameraIsSleeping = false;

    if ((camera->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
    {
        cameraIsSleeping = boost::static_pointer_cast<LivingEntity>(camera)->isSleeping();
    }

    if (entity != camera
        || mMinecraft->mOptions->mThirdPersonView != 0
        || cameraIsSleeping)
    {
        if (entity->isGlowing())
        {
            return true;
        }
    }

    return false;
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

void LevelRenderer::renderEntities(boost::shared_ptr<Entity> camera, Culler* culler, float partialTick)
{
    int userIndex = mMinecraft->mPlayer->mUserIndex;

    PIXBeginNamedEvent(0.0f, "Prepare");

    boost::shared_ptr<Entity> blockEntityDispatcherCamera = mMinecraft->getCameraEntity();
    BlockEntityRenderDispatcher::instance->prepare(mLevels[userIndex], mTextures, mMinecraft->mFont,
        blockEntityDispatcherCamera, mMinecraft->mHitResult, partialTick);

    mMinecraft->getBlockRenderer()->prepare(mLevels[userIndex]);

    boost::shared_ptr<Entity> entityDispatcherCamera = mMinecraft->getCameraEntity();
    mEntityRenderDispatcher->prepare(mLevels[userIndex], mMinecraft->mFont, entityDispatcherCamera,
        mMinecraft->mCameraEntity, mMinecraft->mOptions, partialTick);

    PIXEndNamedEvent();

    if (mSkippedFrames > 0)
    {
        --mSkippedFrames;
        return;
    }

    mCountEntitiesTotal = 0;
    mCountEntitiesRendered = 0;
    mCountEntitiesHidden = 0;

    double camX = (camera->mX - camera->mXo) * partialTick + camera->mXo;
    double camY = (camera->mY - camera->mYo) * partialTick + camera->mYo;
    double camZ = (camera->mZ - camera->mZo) * partialTick + camera->mZo;

    boost::shared_ptr<Entity> positionDispatcherCamera = mMinecraft->getCameraEntity();
    double dispatcherX = (positionDispatcherCamera->mX - positionDispatcherCamera->mXOld) * partialTick +
        positionDispatcherCamera->mXOld;
    double dispatcherY = (positionDispatcherCamera->mY - positionDispatcherCamera->mYOld) * partialTick +
        positionDispatcherCamera->mYOld;
    double dispatcherZ = (positionDispatcherCamera->mZ - positionDispatcherCamera->mZOld) * partialTick +
        positionDispatcherCamera->mZOld;
    BlockEntityRenderDispatcher::xOff = dispatcherX;
    BlockEntityRenderDispatcher::yOff = dispatcherY;
    BlockEntityRenderDispatcher::zOff = dispatcherZ;
    mEntityRenderDispatcher->setPosition(dispatcherX, dispatcherY, dispatcherZ);

    mMinecraft->mGameRenderer->turnOnLightLayer(partialTick);

    std::vector<boost::shared_ptr<Entity> > allEntities = mLevels[userIndex]->getAllEntities();
    mCountEntitiesTotal = (int)allEntities.size();

    PIXBeginNamedEvent(0.0f, "Sort entities by distance for rendering");

    std::vector<std::pair<int, double> > sortedIndices;
    sortedIndices.reserve(allEntities.size());
    for (unsigned int entityIndex = 0; entityIndex < allEntities.size(); ++entityIndex)
    {
        boost::shared_ptr<Entity>& sortEntity = allEntities[entityIndex];
        double dx = camX - sortEntity->mX;
        double dy = camY - sortEntity->mY;
        double dz = camZ - sortEntity->mZ;
        sortedIndices.push_back(std::pair<int, double>((int)entityIndex, dx * dx + dy * dy + dz * dz));
    }
    std::sort(sortedIndices.begin(), sortedIndices.end(), &LevelRenderer::sortEntDisFunc);

    PIXEndNamedEvent();

    InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_Entities);
    PIXBeginNamedEvent(0.0f, "Render %d entities", (int)sortedIndices.size());

    std::vector<boost::shared_ptr<Entity> > glowingEntities;
    std::vector<boost::shared_ptr<Entity> > secondPassEntities;

    for (unsigned int sortIndex = 0; sortIndex < sortedIndices.size(); ++sortIndex)
    {
        boost::shared_ptr<Entity>& entity = allEntities[sortedIndices[sortIndex].first];
        double entityDistanceSqr = sortedIndices[sortIndex].second;

        PIXBeginNamedEvent(0.0f, "shouldRender");
        bool renderable = mEntityRenderDispatcher->shouldRender(entity, culler, entityDistanceSqr);
        if (!renderable)
        {
            if (entity->hasIndirectPassenger(mMinecraft->mPlayer))
            {
                renderable = true;
            }
            else if (entity->hasPassenger(ENTITY_TYPE_PLAYER))
            {
                renderable = true;
            }
        }
        PIXEndNamedEvent();

        if (!renderable)
        {
            continue;
        }

        boost::shared_ptr<Entity> cameraEntity = mMinecraft->getCameraEntity();
        if (entity == cameraEntity)
        {
            bool skip = true;
            if (mMinecraft->mPlayer->GetThirdPersonView(false) != 0)
            {
                skip = false;
            }
            else if ((entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
            {
                if (boost::static_pointer_cast<LivingEntity>(entity)->isSleeping())
                {
                    skip = false;
                }
            }

            if (skip)
            {
                continue;
            }
        }

        if (!(0.0 <= entity->mY && entity->mY < 256.0))
        {
            BlockPos entityBlockPos((int)floor(entity->mX), 0, (int)floor(entity->mZ));
            if (!mLevels[userIndex]->hasChunkAt(entityBlockPos))
            {
                continue;
            }
        }

        ++mCountEntitiesRendered;
        mEntityRenderDispatcher->render(entity, partialTick, false);

        if (shouldEntityGlow(entity, mMinecraft->getCameraEntity(), culler))
        {
            glowingEntities.push_back(entity);
        }

        if (mEntityRenderDispatcher->hasSecondPass(entity))
        {
            secondPassEntities.push_back(entity);
        }
    }

    for (unsigned int secondPassIndex = 0; secondPassIndex < secondPassEntities.size(); ++secondPassIndex)
    {
        mEntityRenderDispatcher->renderSecondPass(secondPassEntities[secondPassIndex], partialTick);
    }

    PIXEndNamedEvent();
    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_Entities);
    Lighting::turnOn();

    EnterCriticalSection(&mBlockEntityMutex);
    PIXBeginNamedEvent(0.0f, "Render %d block entities", (int)mBlockEntities.size());
    InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_BlockEntities);

    std::vector<boost::shared_ptr<BlockEntity> > deferredBlockEntities;

    for (ChunkBlockEntityMap::iterator bucketIt = mBlockEntities.begin(); bucketIt != mBlockEntities.end(); ++bucketIt)
    {
        if (!isGlobalIndexInSameDimension(bucketIt->first, mLevels[userIndex]))
        {
            continue;
        }

        std::vector<boost::shared_ptr<BlockEntity> >& bucket = bucketIt->second;
        for (unsigned int bucketIndex = 0; bucketIndex < bucket.size(); ++bucketIndex)
        {
            boost::shared_ptr<BlockEntity>& blockEntity = bucket[bucketIndex];

            if (blockEntity->GetType() == 0x1200205ULL)
            {
                boost::shared_ptr<PistonMovingBlockEntity> pistonEntity =
                    boost::static_pointer_cast<PistonMovingBlockEntity>(blockEntity);
                if (pistonEntity->getBlockState()->getBlock() == Blocks::slime)
                {
                    deferredBlockEntities.push_back(blockEntity);
                    continue;
                }
            }

            BlockEntityRenderDispatcher::instance->render(blockEntity, partialTick);
        }
    }

    for (unsigned int deferredIndex = 0; deferredIndex < deferredBlockEntities.size(); ++deferredIndex)
    {
        BlockEntityRenderDispatcher::instance->render(deferredBlockEntities[deferredIndex], partialTick);
    }

    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_BlockEntities);
    PIXEndNamedEvent();

    EnterCriticalSection(&mRenderableBlockEntityMutex);
    PIXBeginNamedEvent(0.0f, "Render %d global block entities", (int)mRenderableBlockEntities.size());

    for (boost::unordered_set<boost::shared_ptr<BlockEntity> >::iterator renderableIt = mRenderableBlockEntities.begin();
         renderableIt != mRenderableBlockEntities.end(); ++renderableIt)
    {
        BlockEntityRenderDispatcher::instance->render(*renderableIt, partialTick, -1, true);
    }

    PIXEndNamedEvent();
    LeaveCriticalSection(&mRenderableBlockEntityMutex);

    PIXBeginNamedEvent(0.0f, "Clear out block entities");

    for (ChunkBlockEntityMap::iterator bucketIt = mBlockEntities.begin(); bucketIt != mBlockEntities.end(); ++bucketIt)
    {
        std::vector<boost::shared_ptr<BlockEntity> >& bucket = bucketIt->second;
        for (unsigned int bucketIndex = 0; bucketIndex < bucket.size(); )
        {
            if (bucket[bucketIndex]->shouldRemoveForRender())
            {
                bucket[bucketIndex]->setRenderRemoveStage(0);
                bucket.erase(bucket.begin() + bucketIndex);
            }
            else
            {
                ++bucketIndex;
            }
        }
    }

    setupDestroyState();

    for (boost::unordered_map<int, BlockDestructionProgress*>::iterator progressIt = mDestroyingBlocks.begin();
         progressIt != mDestroyingBlocks.end(); ++progressIt)
    {
        BlockDestructionProgress* progress = progressIt->second;
        BlockPos destroyPos = progress->getPos();
        LevelChunk* destroyChunk = mLevels[userIndex]->getChunkAt(destroyPos);
        boost::shared_ptr<BlockEntity> destroyBlockEntity =
            destroyChunk->getBlockEntity(destroyPos, (LevelChunk::EntityCreationType)2);

        if (!destroyBlockEntity)
        {
            continue;
        }

        if (destroyBlockEntity->GetType() == 0x1e00201ULL)
        {
            boost::shared_ptr<ChestBlockEntity> chestEntity =
                boost::static_pointer_cast<ChestBlockEntity>(destroyBlockEntity);
            boost::shared_ptr<ChestBlockEntity> neighbourChest = chestEntity->mWestNeighbour.lock();

            if (neighbourChest)
            {
                destroyPos = destroyPos.relative(Direction::WEST);
                destroyBlockEntity = mLevels[userIndex]->getBlockEntity(destroyPos);
            }
            else
            {
                neighbourChest = chestEntity->mNorthNeighbour.lock();
                if (neighbourChest)
                {
                    destroyPos = destroyPos.relative(Direction::NORTH);
                    destroyBlockEntity = mLevels[userIndex]->getBlockEntity(destroyPos);
                }
            }
        }

    }

    restoreDestroyState();

    PIXEndNamedEvent();
    LeaveCriticalSection(&mBlockEntityMutex);

    mMinecraft->mGameRenderer->turnOffLightLayer(partialTick);
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

int LevelRenderer::renderChunks(int first, int count, int layer, double partialTick)
{
    int userIndex = mMinecraft->mPlayer->mUserIndex;

    mMinecraft->mGameRenderer->turnOnLightLayer(partialTick);

    boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();

    double camX = (camera->mX - camera->mXo) * partialTick + camera->mXo;
    double camY = (camera->mY - camera->mYo) * partialTick + camera->mYo;
    double camZ = (camera->mZ - camera->mZo) * partialTick + camera->mZo;

    GlStateManager::pushMatrix();
    GlStateManager::translatef(-(float)camX, -(float)camY, -(float)camZ);

    if (layer == 2)
    {
        unsigned int waterColor = Biome::OCEAN->getWaterColor();
        float alpha = (float)((waterColor >> 24) & 0xff) / 255.0f;
        gRenderer->StateSetVertexAlpha(alpha);
    }

    bool running = true;
    unsigned int mask = (unsigned int)(4 << (layer & 0x3f)) & 0xff;

    setupCameraInsideAreas(camera);

    std::vector<unsigned int> visibleChunks;
    visibleChunks.reserve(mClipChunks[userIndex].length);

    ClipChunk* chunks = mClipChunks[userIndex].data;
    int visibleCount = 0;

    for (unsigned int index = 0; index < mClipChunks[userIndex].length; ++index)
    {
        ClipChunk* chunk = &chunks[index];

        if (chunk->mVisible && chunk->mGlobalIndex != -1 &&
            (mask & mGlobalChunkFlags[chunk->mGlobalIndex]) != mask &&
            clippedByCameraInsideAreas(chunk) == 0)
        {
            unsigned int bucket = getBucketMask(camX, camY, camZ, chunk);
            visibleChunks.push_back((unsigned int)chunk->mGlobalIndex | (bucket << 24));
            ++visibleCount;
        }
    }

    for (std::vector<unsigned int>::iterator entry = visibleChunks.begin(); entry != visibleChunks.end(); ++entry)
    {
        int displayList = (int)(*entry & 0xffffff) * 3 + layer + mDisplayList;
        unsigned char layerMask = (unsigned char)(*entry >> 24);

        if (gRenderer->CBuffCall(displayList, running, layerMask))
        {
            running = false;
        }
    }

    GlStateManager::popMatrix();
    mMinecraft->mGameRenderer->turnOffLightLayer(partialTick);

    return visibleCount;
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

void LevelRenderer::renderSky(float partialTick)
{
    if (mMinecraft->mLevel->mDimension->getType() == DimensionType::THE_END)
    {
        GlStateManager::disableFog();
        GlStateManager::disableAlphaTest();
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        Lighting::turnOff();
        GlStateManager::depthMask(false);

        static ResourceLocation sEndSkyLocation;
        mTextures->bindTexture(&sEndSkyLocation);

        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        builder->setMipmapEnable(false);

        static const float sVoidBoxRotations[6][4] =
        {
            { 0.0f, 0.0f, 0.0f, 0.0f },
            { 90.0f, 1.0f, 0.0f, 0.0f },
            { -90.0f, 1.0f, 0.0f, 0.0f },
            { 180.0f, 1.0f, 0.0f, 0.0f },
            { 90.0f, 0.0f, 0.0f, 1.0f },
            { -90.0f, 0.0f, 0.0f, 1.0f },
        };

        for (int face = 0; face < 6; ++face)
        {
            GlStateManager::pushMatrix();

            if (face != 0)
            {
                GlStateManager::rotatef(sVoidBoxRotations[face][0], sVoidBoxRotations[face][1],
                                        sVoidBoxRotations[face][2], sVoidBoxRotations[face][3]);
            }

            builder->begin();
            builder->color(0x282828);
            builder->vertexUV(-100.0f, -100.0f, -100.0f, 0.0f, 0.0f);
            builder->vertexUV(-100.0f, -100.0f, 100.0f, 0.0f, 16.0f);
            builder->vertexUV(100.0f, -100.0f, 100.0f, 16.0f, 16.0f);
            builder->vertexUV(100.0f, -100.0f, -100.0f, 16.0f, 0.0f);
            builder->end();

            GlStateManager::popMatrix();
        }

        builder->setMipmapEnable(true);
        GlStateManager::depthMask(true);
        GlStateManager::enableTexture();
        GlStateManager::enableAlphaTest();
        return;
    }

    bool biomeNotNatural = false;

    if (mMinecraft->mPlayer)
    {
        Biome* biome = mMinecraft->mLevel->getBiome(BlockPos(mMinecraft->mPlayer));

        if (!biome->isNatural())
        {
            biomeNotNatural = true;
        }
    }

    if (!mMinecraft->mLevel->mDimension->isNaturalDimension() || biomeNotNatural)
    {
        return;
    }

    int userIndex = mMinecraft->mPlayer->mUserIndex;
    Level* level = mLevels[userIndex];

    if (!level->getGameRules()->getBoolean(8))
    {
        partialTick = 0.0f;
    }

    boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
    Vec3* skyColor = level->getSkyColor(camera, partialTick);
    float red = (float)skyColor->x;
    float green = (float)skyColor->y;
    float blue = (float)skyColor->z;

    if (mMinecraft->mOptions->getBooleanValue(Options::Option::anaglyph))
    {
        float red30 = red * 30.0f;
        float newBlue = (blue * 70.0f + red30) / 100.0f;
        float newRed = (blue * 11.0f + (green * 59.0f + red30)) / 100.0f;
        float newGreen = (green * 70.0f + red30) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }

    GlStateManager::color3f(red, green, blue);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    GlStateManager::depthMask(false);
    GlStateManager::enableFog();
    GlStateManager::color3f(red, green, blue);
    GlStateManager::callList(mSkyList);
    GlStateManager::disableFog();
    GlStateManager::disableAlphaTest();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    Dimension* dimension = level->mDimension;
    float* sunriseColors = dimension->getSunriseColor((float)level->getTimeOfDay(partialTick), partialTick);

    if (sunriseColors != 0)
    {
        GlStateManager::disableTexture();
        GlStateManager::shadeModel(0);
        GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
        GlStateManager::pushMatrix();
        GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);

        float faceAngle = (Mth::sin(level->getSunAngle(partialTick)) >= 0.0f) ? 0.0f : 180.0f;
        GlStateManager::rotatef(faceAngle, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(90.0f, 0.0f, 0.0f, 1.0f);

        float sunriseR = sunriseColors[0];
        float sunriseG = sunriseColors[1];
        float sunriseB = sunriseColors[2];

        if (mMinecraft->mOptions->getBooleanValue(Options::Option::anaglyph))
        {
            float r30 = sunriseR * 30.0f;
            float newB = (sunriseB * 70.0f + r30) / 100.0f;
            float newG = (sunriseG * 70.0f + r30) / 100.0f;
            float newR = (sunriseB * 11.0f + (sunriseG * 59.0f + r30)) / 100.0f;
            sunriseR = newR;
            sunriseG = newG;
            sunriseB = newB;
        }

        builder->begin(2);
        builder->color(sunriseR, sunriseG, sunriseB, sunriseColors[3]);
        builder->vertex(0.0f, 100.0f, 0.0f);
        builder->color(sunriseColors[0], sunriseColors[1], sunriseColors[2], 0.0f);

        for (int segment = 0; segment <= 16; ++segment)
        {
            float angle = (float)segment * 3.14159274f / 8.0f;
            float sinA = Mth::sin(angle);
            float cosA = Mth::cos(angle);
            builder->vertex(sinA * 120.0f, cosA * 120.0f, -(cosA * 40.0f * sunriseColors[3]));
        }

        builder->end();
        GlStateManager::popMatrix();
        GlStateManager::shadeModel(0);
    }

    float sunAngleDegrees = (float)(level->getSunAngle(partialTick) * 57.2957763671875);
    float moonAngleDegrees = fmodf(sunAngleDegrees + 180.0f, 360.0f);

    GlStateManager::enableTexture();
    GlStateManager::blendFunc(4, 1);
    GlStateManager::pushMatrix();

    float rainLevel = level->getRainLevel(partialTick);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f - rainLevel);
    GlStateManager::translatef(0.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(level->getTimeOfDay(partialTick) * 360.0f, 1.0f, 0.0f, 0.0f);

    static ResourceLocation sSunLocation;
    static ResourceLocation sMoonPhasesLocation;

    if (sunAngleDegrees <= 125.0f || 235.0f <= sunAngleDegrees)
    {
        MemSect(0x1f);
        mTextures->bindTexture(&sSunLocation);
        MemSect(0);

        builder->begin();
        builder->vertexUV(-30.0f, 100.0f, -30.0f, 1.0f, 1.0f);
        builder->vertexUV(30.0f, 100.0f, -30.0f, 0.0f, 1.0f);
        builder->vertexUV(30.0f, 100.0f, 30.0f, 0.0f, 0.0f);
        builder->vertexUV(-30.0f, 100.0f, 30.0f, 1.0f, 0.0f);
        builder->end();
    }

    if (moonAngleDegrees <= 125.0f || 235.0f <= moonAngleDegrees)
    {
        mTextures->bindTexture(&sMoonPhasesLocation);

        int moonPhase = level->getMoonPhase();
        int column = moonPhase & 3;
        int row = (moonPhase >> 2) % 2;
        float u0 = (float)column * 0.25f;
        float u1 = (float)(column + 1) * 0.25f;
        float v0 = (float)row * 0.5f;
        float v1 = (float)(row + 1) * 0.5f;

        builder->begin();
        builder->vertexUV(-20.0f, -100.0f, 20.0f, u1, v1);
        builder->vertexUV(20.0f, -100.0f, 20.0f, u0, v1);
        builder->vertexUV(20.0f, -100.0f, -20.0f, u0, v0);
        builder->vertexUV(-20.0f, -100.0f, -20.0f, u1, v0);
        builder->end();
    }

    GlStateManager::disableTexture();

    float starBrightness = level->getStarBrightness(partialTick) * (1.0f - rainLevel);

    if (starBrightness > 0.0f)
    {
        GlStateManager::color4f(starBrightness, starBrightness, starBrightness, starBrightness);
        GlStateManager::callList(mStarList);
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::enableAlphaTest();
    GlStateManager::enableFog();
    GlStateManager::popMatrix();
    GlStateManager::disableTexture();
    GlStateManager::color3f(0.0f, 0.0f, 0.0f);

    if (!dimension->hasGround())
    {
        GlStateManager::color3f(red, green, blue);
        GlStateManager::enableTexture();
        GlStateManager::depthMask(true);
    }
    else
    {
        GlStateManager::color3f(red * 0.2f + 0.04f, green * 0.2f + 0.04f, blue * 0.6f + 0.1f);
        GlStateManager::enableTexture();
        GlStateManager::depthMask(true);
    }
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

void LevelRenderer::renderAdvancedClouds(float partialTick, int pass, double x, double y, double z)
{
    static float sCloudDriftDelta = 0.0f;
    static float sCloudDriftAccum = 0.0f;
    static float sCloudPrevTime = -1.0f;
    static bool sCloudDriftLatched = false;
    static ResourceLocation sAdvancedCloudsLocation;

    GlStateManager::multiTexCoord2f(0.0f, 0.0f);
    gRenderer->StateSetEnableViewportClipPlanes(true);

    int userIndex = mMinecraft->mPlayer->mUserIndex;
    Level* level = mLevels[userIndex];

    float currentTime = (float)mRenderTick + partialTick;

    if (sCloudDriftDelta != 0.0f)
    {
        sCloudDriftAccum = sCloudDriftAccum + sCloudDriftDelta;
    }

    double driftAccum = (double)sCloudDriftAccum;
    double cellX;
    double cellZ;

    if (!sCloudDriftLatched)
    {
        if (sCloudPrevTime != -1.0f)
        {
            sCloudDriftAccum = (float)(driftAccum + ((double)currentTime - (double)sCloudPrevTime));
            driftAccum = (double)sCloudDriftAccum;
            sCloudPrevTime = -1.0f;
        }

        cellZ = z / 12.0 + 0.33000001311302185;
        cellX = (((double)currentTime - driftAccum) * 1.2399998903274536 + x) / 12.0;
    }
    else
    {
        if (sCloudPrevTime == -1.0f)
        {
            sCloudPrevTime = currentTime;
        }

        cellZ = z / 12.0 + 0.33000001311302185;
        cellX = (((double)sCloudPrevTime - driftAccum) * 1.2399998903274536 + x) / 12.0;
    }

    double cloudRelativeY = level->mDimension->getCloudHeight();
    cloudRelativeY = (double)(float)(cloudRelativeY - (double)(float)y) + 0.33000001311302185;

    int sectorX = (int)Mth::lfloor(cellX * (1.0 / 2048.0));
    int sectorZ = (int)Mth::lfloor(cellZ * (1.0 / 2048.0));
    cellX -= (double)(sectorX << 11);
    cellZ -= (double)(sectorZ << 11);

    bool cameraInCloudBand = (-5.0 < cloudRelativeY) && (cloudRelativeY <= 5.0);

    if (cameraInCloudBand)
    {
        GlStateManager::disableCull();
    }
    else
    {
        GlStateManager::enableCull();
    }

    MemSect(0x1f);
    mTextures->bindTexture(&sAdvancedCloudsLocation);
    MemSect(0);

    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    Vec3* cloudColor = level->getCloudColor(partialTick);
    float red = (float)cloudColor->x;
    float green = (float)cloudColor->y;
    float blue = (float)cloudColor->z;
    float alpha = 1.0f;

    if (mMinecraft->mOptions->getBooleanValue(Options::Option::anaglyph))
    {
        float red30 = red * 30.0f;
        float newBlue = (blue * 70.0f + red30) / 100.0f;
        float newRed = (blue * 11.0f + (green * 59.0f + red30)) / 100.0f;
        float newGreen = (green * 70.0f + red30) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }

    long long cellAnchorX = Mth::lfloor(cellX);
    long long cellAnchorZ = Mth::lfloor(cellZ);
    float fracX = (float)(cellX - (double)cellAnchorX);
    float fracZ = (float)(cellZ - (double)cellAnchorZ);

    float wrapAnchorX = (float)cellAnchorX * (1.0f / 256.0f);
    float wrapAnchorZ = (float)cellAnchorZ * (1.0f / 256.0f);

    while (wrapAnchorX < 1.0f)
    {
        wrapAnchorX = wrapAnchorX + 1.0f;
    }

    while (wrapAnchorZ < 1.0f)
    {
        wrapAnchorZ = wrapAnchorZ + 1.0f;
    }

    int gridRadius = 3;

    if (activePlayers() > 2)
    {
        gridRadius = 2;
    }

    GlStateManager::scalef(12.0f, 1.0f, 12.0f);
    FrustumData* frustum = Frustum::getFrustum();

    int gridMin = 1 - gridRadius;

    for (int blendPass = 0; blendPass < 2; ++blendPass)
    {
        if (blendPass == 0)
        {
            GlStateManager::blendFunc(0, 1);
        }
        else
        {
            GlStateManager::blendFunc(4, 5);
        }

        if (gridMin <= gridRadius)
        {
            for (int cellRow = gridMin; cellRow <= gridRadius; ++cellRow)
            {
                float rowWorld = (float)(cellRow * 8);

                for (int cellCol = gridMin; cellCol <= gridRadius; ++cellCol)
                {
                    float colWorld = (float)(cellCol * 8);
                    double boxX = (double)(rowWorld - fracX);
                    double boxZ = (double)(colWorld - fracZ);

                    if (frustum->cubeInFrustum(boxX, cloudRelativeY, boxZ,
                                                boxX + 8.0, cloudRelativeY + 4.0, boxZ + 8.0))
                    {
                        GlStateManager::matrixMode(2);
                        GlStateManager::loadIdentity();
                        GlStateManager::translatef(rowWorld * (1.0f / 256.0f) + wrapAnchorX,
                                                    colWorld * (1.0f / 256.0f) + wrapAnchorZ, 0.0f);
                        GlStateManager::matrixMode(0);
                        GlStateManager::pushMatrix();
                        GlStateManager::translatef((float)boxX, (float)cloudRelativeY, (float)boxZ);
                        GlStateManager::color4f(red, green, blue, alpha);

                        if (!cameraInCloudBand)
                        {
                            GlStateManager::callList(mCloudLists + 6);
                        }
                        else
                        {
                            GlStateManager::callList(mCloudLists + 0);
                            GlStateManager::callList(mCloudLists + 1);

                            if (cellRow >= 0)
                            {
                                if (cellRow < 2)
                                {
                                    GlStateManager::translatef(0.0009765625f, 0.0f, 0.0f);
                                    GlStateManager::callList(mCloudLists + 2);
                                    GlStateManager::translatef(-0.0009765625f, 0.0f, 0.0f);
                                    GlStateManager::callList(mCloudLists + 3);
                                }
                                else
                                {
                                    GlStateManager::callList(mCloudLists + 2);
                                }
                            }
                            else
                            {
                                GlStateManager::callList(mCloudLists + 3);
                            }

                            if (cellCol >= 0)
                            {
                                if (cellCol < 2)
                                {
                                    GlStateManager::translatef(0.0f, 0.0f, 0.0009765625f);
                                    GlStateManager::callList(mCloudLists + 4);
                                    GlStateManager::translatef(0.0f, 0.0f, -0.0009765625f);
                                    GlStateManager::callList(mCloudLists + 5);
                                }
                                else
                                {
                                    GlStateManager::callList(mCloudLists + 4);
                                }
                            }
                            else
                            {
                                GlStateManager::callList(mCloudLists + 5);
                            }
                        }

                        GlStateManager::popMatrix();
                        GlStateManager::matrixMode(2);
                        GlStateManager::loadIdentity();
                        GlStateManager::matrixMode(0);
                    }
                }
            }
        }
    }

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::enableCull();

    gRenderer->StateSetEnableViewportClipPlanes(false);
    instructionSynchronize();
}

void LevelRenderer::renderClouds(float partialTick, int pass, double x, double y, double z)
{
    if (!GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), (eGameSetting)0x12))
    {
        return;
    }

    bool biomeNotNatural = false;

    if (mMinecraft->mPlayer)
    {
        Biome* biome = mMinecraft->mLevel->getBiome(BlockPos(mMinecraft->mPlayer));

        if (!biome->isNatural())
        {
            biomeNotNatural = true;
        }
    }

    if (!mMinecraft->mLevel->mDimension->isNaturalDimension() || biomeNotNatural)
    {
        return;
    }

    if (mMinecraft->mOptions->mFancyGraphics)
    {
        renderAdvancedClouds(partialTick, pass, x, y, z);
        return;
    }

    GlStateManager::disableCull();

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    static ResourceLocation sCloudsLocation;
    mTextures->bindTexture(&sCloudsLocation);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    int userIndex = mMinecraft->mPlayer->mUserIndex;
    Level* level = mLevels[userIndex];
    Vec3* cloudColor = level->getCloudColor(partialTick);
    float red = (float)cloudColor->x;
    float green = (float)cloudColor->y;
    float blue = (float)cloudColor->z;

    if (mMinecraft->mOptions->getBooleanValue(Options::Option::anaglyph))
    {
        float red30 = red * 30.0f;
        float newBlue = (blue * 70.0f + red30) / 100.0f;
        float newRed = (blue * 11.0f + (green * 59.0f + red30)) / 100.0f;
        float newGreen = (green * 70.0f + red30) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }

    x = ((float)mRenderTick + partialTick) * 1.2399999f + x;

    int gridX = Mth::lfloor(x * 0.75);
    int gridZ = Mth::lfloor(z * 0.75);
    float localX = (float)((x - (double)(gridX << 11)) * 0.75);
    float localZ = (float)((z - (double)(gridZ << 11)) * 0.75);
    float cloudY = (float)(level->mDimension->getCloudHeight() - (double)(float)y + 0.33f);

    builder->begin();
    builder->color(red, green, blue, 0.8f);

    for (int cellX = 0; cellX < 16; ++cellX)
    {
        float x0 = -256.0f + (float)cellX * 32.0f;
        float x1 = x0 + 32.0f;
        float u0 = x0 * 0.00048828125f + localX;
        float u1 = x1 * 0.00048828125f + localX;

        for (int cellZ = 0; cellZ < 16; ++cellZ)
        {
            float z0 = -256.0f + (float)cellZ * 32.0f;
            float z1 = z0 + 32.0f;
            float v0 = z0 * 0.00048828125f + localZ;
            float v1 = z1 * 0.00048828125f + localZ;

            builder->vertexUV(x0, cloudY, z0, u0, v0);
            builder->vertexUV(x1, cloudY, z0, u1, v0);
            builder->vertexUV(x1, cloudY, z1, u1, v1);
            builder->vertexUV(x0, cloudY, z1, u0, v1);
        }
    }

    builder->end();

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::enableCull();
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

unsigned int LevelRenderer::updateDirtyChunks(unsigned int* count)
{
    std::vector<Chunk*> emptyChunks;

    ClipChunk* nearest = 0;
    int nearestDistance = 0x7fffffff;
    int nearCount = 0;

    unsigned int commandBufferSize = (unsigned int)gRenderer->CBuffSize(-1);
    float commandBufferBytes = (float)commandBufferSize;

    InfoBarManager::SetCommandBufferSize(commandBufferBytes / 115343360.0f);
    PIXAddNamedCounter((float)(int)(commandBufferBytes / 1048576.0f),
                       "Command buffer allocations");

    EnterCriticalSection(&mChunkMutex);

    int* dirtyChunk;

    while ((dirtyChunk = mDirtyChunkStack.Pop()) != 0)
    {
        if ((int)dirtyChunk > 1)
        {
            setGlobalChunkFlag((int)dirtyChunk - 2, 2, 0);
        }

        mHasDirtyChunks = true;
    }

    if (mHasDirtyChunks)
    {
        mLastRebuildTime = System::processTimeInMilliSecs();

        PIXBeginNamedEvent(0.0f, "Finding nearest chunk\n");

        for (int player = 0; player < 4; ++player)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
                mMinecraft->mLocalPlayers[player];

            if (!localPlayer)
            {
                continue;
            }

            if (mClipChunks[player].data == 0)
            {
                continue;
            }

            if (mLevels[player] == 0)
            {
                continue;
            }

            if ((int)mClipChunks[player].length != mChunkGridWidth * mChunkGridDepth * 16)
            {
                continue;
            }

            int cameraX = (int)localPlayer->mX;
            int cameraY = (int)localPlayer->mY;
            int cameraZ = (int)localPlayer->mZ;

            for (int gridZ = 0; gridZ < mChunkGridDepth; ++gridZ)
            {
                for (int gridY = 0; gridY < 16; ++gridY)
                {
                    for (int gridX = 0; gridX < mChunkGridWidth; ++gridX)
                    {
                        ClipChunk* clipChunk =
                            &mClipChunks[player]
                                 .data[(gridZ * mChunkGridHeight + gridY) * mChunkGridWidth +
                                       gridX];

                        int dx = clipChunk->mX - cameraX;
                        int dy = clipChunk->mY - cameraY;
                        int dz = clipChunk->mZ - cameraZ;
                        int globalIndex = clipChunk->mGlobalIndex;
                        int distance = dx * dx + dy * dy + dz * dz;
                        int squishedDistance = dx * dx + dy * dy * 4 + dz * dz;

                        if (globalIndex == -1)
                        {
                            continue;
                        }

                        unsigned char flags = mGlobalChunkFlags[globalIndex];

                        if ((flags & 2) == 0)
                        {
                            continue;
                        }

                        if (commandBufferSize >= 115343360u && (flags & 1) == 0 && distance >= 400)
                        {
                            continue;
                        }

                        if (squishedDistance < nearestDistance)
                        {
                            Chunk* chunk = clipChunk->mChunk;
                            LevelChunk* levelChunk = mLevels[player]->getChunkAt(
                                BlockPos(chunk->mX, 0, chunk->mDimension));

                            if (!levelChunk->isRenderChunkEmpty(gridY * 16))
                            {
                                nearestDistance = squishedDistance;
                                nearest = clipChunk;
                            }
                            else
                            {
                                chunk->clearDirty();
                                mGlobalChunkFlags[globalIndex] |= 0x1c;
                                emptyChunks.push_back(chunk);
                            }
                        }

                        if (distance < 400)
                        {
                            ++nearCount;
                        }
                    }
                }
            }
        }

        PIXEndNamedEvent();
    }

    if (nearest == 0)
    {
        long long elapsed = (long long)System::processTimeInMilliSecs() - mLastRebuildTime;

        mHasDirtyChunks = elapsed > 250;

        LeaveCriticalSection(&mChunkMutex);

        return 0;
    }

    Chunk* chunk = nearest->mChunk;

    PIXBeginNamedEvent(0.0f, "Rebuilding near chunk %d %d %d", chunk->mX, chunk->mZ,
                       chunk->mDimension);

    if (nearCount > 0)
    {
        gRenderer->CBuffDeferredModeStart();
    }

    chunk->clearDirty();

    static Chunk rebuildChunk;

    rebuildChunk.makeCopyForRebuild(chunk);

    LeaveCriticalSection(&mChunkMutex);

    rebuildChunk.rebuild(count);

    PIXEndNamedEvent();

    for (int remaining = (int)emptyChunks.size(); remaining > 0; --remaining)
    {
        Chunk* emptyChunk = emptyChunks[remaining - 1];

        mDestroyedBlocks->updatedChunkAt(
            emptyChunk->mLevel,
            BlockPos(emptyChunk->mX, emptyChunk->mZ, emptyChunk->mDimension), remaining);
    }

    if (nearCount > 1)
    {
        mDestroyedBlocks->updatedChunkAt(chunk->mLevel,
                                         BlockPos(chunk->mX, chunk->mZ, chunk->mDimension),
                                         nearCount);

        return 1;
    }

    if (nearCount > 0 &&
        getGlobalChunkFlag(chunk->mX, chunk->mZ, chunk->mDimension, chunk->mLevel, 2, 0))
    {
        mDestroyedBlocks->updatedChunkAt(chunk->mLevel,
                                         BlockPos(chunk->mX, chunk->mZ, chunk->mDimension), 2);

        return 1;
    }

    mDestroyedBlocks->updatedChunkAt(chunk->mLevel,
                                     BlockPos(chunk->mX, chunk->mZ, chunk->mDimension), nearCount);

    return 0;
}

static ResourceLocation sHitOverlayLocation;

void LevelRenderer::renderHit(boost::shared_ptr<Player> player, const HitResult* hit, int pass, not_null_ptr<ItemInstance> item, float partialTick)
{
    GlStateManager::enableBlend();
    GlStateManager::enableAlphaTest();
    GlStateManager::blendFunc(4, 1);

    float phase = sinf((float)((double)Minecraft::currentTimeMillis() / 100.0));
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, (phase * 0.20000000298023224f + 0.4000000059604645f) * 0.5f);

    if (pass != 0)
    {
        if (!item->isEmpty())
        {
            GlStateManager::blendFunc(4, 5);

            float phase1 = sinf((float)((double)Minecraft::currentTimeMillis() / 100.0));
            float rgb = phase1 * 0.20000000298023224f + 0.800000011920929f;

            float phase2 = sinf((float)((double)Minecraft::currentTimeMillis() / 200.0));
            float alpha = phase2 * 0.20000000298023224f + 0.5f;

            GlStateManager::color4f(rgb, rgb, rgb, alpha);
            mTextures->bindTexture(&sHitOverlayLocation);
        }
    }

    GlStateManager::disableBlend();
    GlStateManager::disableAlphaTest();
}

void LevelRenderer::renderDestroyAnimation(Tesselator* tesselator, boost::shared_ptr<Player> player, float partialTick)
{
    double camX = (player->mX - player->mXo) * partialTick + player->mXo;
    double camY = (player->mY - player->mYo) * partialTick + player->mYo;
    double camZ = (player->mZ - player->mZo) * partialTick + player->mZo;
    int userIndex = mMinecraft->mPlayer->mUserIndex;

    if (mDestroyingBlocks.empty())
    {
        return;
    }

    BufferBuilder* builder = tesselator->getBuilder();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(8, 2);
    mTextures->bindTexture(&sHitOverlayLocation);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 0.5f);
    GlStateManager::pushMatrix();
    GlStateManager::disableAlphaTest();
    GlStateManager::polygonOffset(-3.0f, -3.0f);
    GlStateManager::enablePolygonOffset();
    GlStateManager::enableAlphaTest();
    builder->begin();
    builder->offset(-(float)camX, -(float)camY, -(float)camZ);
    builder->noColor();

    for (boost::unordered_map<int, BlockDestructionProgress*>::iterator entry = mDestroyingBlocks.begin();
         entry != mDestroyingBlocks.end(); ++entry)
    {
        BlockDestructionProgress* progress = entry->second;
        BlockPos pos = progress->getPos();
        double dx = (double)pos.x - camX;
        double dy = (double)pos.y - camY;
        double dz = (double)pos.z - camZ;

        if (dx * dx + dy * dy + dz * dz < 1024.0)
        {
            const BlockState* state = mMinecraft->mLevel->getBlockState(pos);
            state->getBlock();

            mBlockRenderers[userIndex]->tesselateInWorldFixedTexture(
                mMinecraft->mLevel->getBlockState(pos), pos, mDestroyStages[progress->getProgress()]);
        }
    }

    builder->end();
    builder->offset(0.0f, 0.0f, 0.0f);
    GlStateManager::disableAlphaTest();
    GlStateManager::polygonOffset(0.0f, 0.0f);
    GlStateManager::disablePolygonOffset();
    GlStateManager::enableAlphaTest();
    GlStateManager::depthMask(true);
    GlStateManager::popMatrix();
    GlStateManager::disableBlend();
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

void LevelRenderer::renderHitOutline(boost::shared_ptr<Player> player, const HitResult* hit, int pass, float partialTick)
{
    int userIndex = mMinecraft->mPlayer->mUserIndex;

    if (pass == 0 && hit->mType == HitResult::BLOCK &&
        GameSettings::GetGameSettings(userIndex, (eGameSetting)0x17))
    {
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        GlStateManager::color4f(0.0f, 0.0f, 0.0f, 0.4000000059604645f);
        GlStateManager::lineWidth(2.0f);
        GlStateManager::disableTexture();
        GlStateManager::depthMask(false);

        const BlockState* state = mMinecraft->mLevel->getBlockState(hit->getBlockPos());
        Block* block = state->getBlock();

        double camX = (player->mX - player->mXo) * partialTick + player->mXo;
        double camY = (player->mY - player->mYo) * partialTick + player->mYo;
        double camZ = (player->mZ - player->mZo) * partialTick + player->mZo;

        AABB* box = block->getOutlineAABB(state, mLevels[userIndex], hit->getBlockPos())
                        ->grow(0.0020000000949949026, 0.0020000000949949026, 0.0020000000949949026)
                        ->move(-camX, -camY, -camZ);

        renderLineBox(box, 0xffffff);

        GlStateManager::depthMask(true);
        GlStateManager::enableTexture();
        GlStateManager::disableBlend();
    }
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

void LevelRenderer::notifyNearbyEntities(Level* level, const BlockPos& pos, bool playing)
{
    const AABB* box = AABB::newTemp(pos)->grow(3.0);
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntitiesOfClass(typeid(LivingEntity), box);

    if (entities != NULL)
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(*it);

            livingEntity->setRecordPlayingNearby(pos, playing);
        }

        delete entities;
    }
}

void LevelRenderer::cull(Culler* culler, float partialTick)
{
    boost::shared_ptr<Entity> cameraEntity = mMinecraft->getCameraEntity();
    int userIndex = boost::static_pointer_cast<LocalPlayer>(cameraEntity)->mUserIndex;

    FrustumCuller* frustumCuller = reinterpret_cast<FrustumCuller*>(culler);
    double xOffset = -frustumCuller->xOffset;
    double yOffset = -frustumCuller->yOffset;
    double zOffset = -frustumCuller->zOffset;
    float** frustum = frustumCuller->frustumData->frustum;

    float planes[24];

    for (int i = 0; i < 6; ++i)
    {
        float* plane = frustum[i];
        planes[i * 4 + 0] = plane[0];
        planes[i * 4 + 1] = plane[1];
        planes[i * 4 + 2] = plane[2];
        planes[i * 4 + 3] = (float)((double)plane[2] * zOffset + (double)plane[1] * yOffset +
                                    (double)plane[0] * xOffset + (double)plane[3]);
    }

    ClipChunk* chunks = mClipChunks[userIndex].data;
    unsigned int count = mClipChunks[userIndex].length;

    for (unsigned int i = 0; i < count; ++i)
    {
        ClipChunk* chunk = &chunks[i];

        if (chunk->mGlobalIndex == -1)
        {
            chunk->mVisible = false;
            continue;
        }

        unsigned char flags = mGlobalChunkFlags[chunk->mGlobalIndex];

        if ((flags & 1) == 0 || (flags & 0x1c) == 0x1c)
        {
            chunk->mVisible = false;
            continue;
        }

        chunk->mVisible = clip(&chunk->mMinX, planes);
    }
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

int LevelRenderer::checkAllPresentChunks(bool* present)
{
    if (present == NULL || mMinecraft == NULL)
        return 0;

    boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
    if (camera == NULL)
        return 0;

    int userIndex = boost::static_pointer_cast<LocalPlayer>(camera)->mUserIndex;
    Level* level = mLevels[userIndex];
    ClipChunk* chunks = mClipChunks[userIndex].data;
    unsigned int count = mClipChunks[userIndex].length;
    int missing = 0;
    *present = false;

    for (unsigned int i = 0; i < count; ++i)
    {
        ClipChunk& clip = chunks[i];
        if (clip.mChunk != NULL && (level == NULL ||
                                    !level->reallyHasChunk(clip.mChunk->mX >> 4, clip.mChunk->mZ >> 4)))
        {
            ++missing;
            for (int x = 4; x < 13; ++x)
            {
                for (int z = 4; z <= 12; z += 4)
                {
                    for (int dx = 0; dx < 4; ++dx)
                    {
                        BlockPos neighbor(clip.mX + x, 0, clip.mZ + z + dx);
                        const BlockState* state = level == NULL ? NULL : level->getBlockState(neighbor);
                        const BlockState* dirtState = Blocks::dirt->defaultBlockState();
                        const BlockState* bedrockState = Blocks::bedrock->defaultBlockState();
                        if (state != bedrockState && state != dirtState)
                            *present = true;
                    }
                }
            }
        }
    }
    return missing;
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
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);

        player->prepareCustomTextures();
    }
}

void LevelRenderer::entityRemoved(boost::shared_ptr<Entity> entity)
{
    if (!entity)
        return;
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) != ENTITY_TYPE_PLAYER)
        return;
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    if (!player)
        return;
    if (!player->mSkinLocation.empty())
        mTextures->removeMemTexture(player->mSkinLocation);
    if (!player->mCapeLocation.empty())
        mTextures->removeMemTexture(player->mCapeLocation);
}

void LevelRenderer::globalLevelEvent(int type, const BlockPos& pos, int data)
{
    if (mMinecraft == NULL || mMinecraft->mLevel == NULL || mMinecraft->mLevel->mDimension == NULL)
        return;

    int dimension = getDimensionIndexFromId(mMinecraft->mLevel->mDimension->getType()->getId());
    Level* level = mLevels[dimension];
    if (level == NULL || mMinecraft->mParticleEngine == NULL)
        return;

    boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
    if (camera == NULL)
        return;

    double x = (double)pos.x - camera->mX;
    double y = (double)pos.y - camera->mY;
    double z = (double)pos.z - camera->mZ;
    double distance = std::sqrt(x * x + y * y + z * z);
    if (distance > 176.0)
    {
        x += x / distance * 1.4;
        y += y / distance * 1.4;
        z += z / distance * 1.4;
    }

    if (type == 1023)
    {
        addParticle(level, PARTICLE_HUGE_EXPLOSION, camera->mX + x,
                    camera->mY + y, camera->mZ + z, 0.0078125, 0.0, 0.0,
                    arrayWithLength<int>());
    }
    else if (type == 1038)
    {
        addParticle(level, PARTICLE_WINNING_INSTANT_SPELL, camera->mX + x,
                    camera->mY + y, camera->mZ + z, 0.0078125, 0.0, 0.0,
                    arrayWithLength<int>());
    }
    else if (type == 1028)
    {
        addParticle(level, PARTICLE_ENDER, camera->mX + x,
                    camera->mY + y, camera->mZ + z, 0.0078125, -4.0, 16.0,
                    arrayWithLength<int>());
    }
}

void LevelRenderer::levelEvent(boost::shared_ptr<Player> player, int type, const BlockPos& pos, int data)
{
    if (mMinecraft == NULL) return;
    Level* world = mMinecraft->mLevel;
    if (world == NULL || world->mDimension == NULL) return;
    int dimension = getDimensionIndexFromId(world->mDimension->getType()->getId());
    Level* level = mLevels[dimension];
    if (level == NULL) return;
    Random* random = level->mRandom;

    const auto playBlockEventSound = [level, &pos](const SoundEvent* sound, float volume,
                                                    float pitch, float range)
    {
        if (sound != NULL)
        {
            level->playLocalSound((double)pos.x, (double)pos.y, (double)pos.z, sound,
                                  SoundSource::eSoundSource_Blocks, volume, pitch, false, range);
        }
    };

    const auto playLevelEventParticle = [this, level, &pos](const ParticleType* particle,
                                                              double xa, double ya, double za,
                                                              double offset)
    {
        if (particle != NULL)
            addParticle(level, particle, pos.x + offset, pos.y + offset, pos.z + offset,
                        xa, ya, za, arrayWithLength<int>());
    };

    if (type >= 2048 && type <= 3000)
    {
        const ParticleType* particle = ParticleType::byId(static_cast<ePARTICLE_TYPE>(data));
        if (particle != NULL)
        {
            arrayWithLength<int> params(1, false);
            params.data[0] = data;
            addParticle(level, particle, pos.x + 0.5, pos.y + 0.5, pos.z + 0.5,
                        0.0, 0.0, 0.0, params);
        }
        return;
    }

    switch (type)
    {
    case 1000: playLevelEventParticle(PARTICLE_SMOKE, 0.0, 0.0, 0.0, 0.0); break;
    case 1001: playLevelEventParticle(PARTICLE_SMOKE, 0.0, 0.0, 0.0, 0.0); break;
    case 1002: playLevelEventParticle(PARTICLE_SMOKE, 0.0, 0.0, 0.0, 0.0); break;
    case 1003: playLevelEventParticle(PARTICLE_SMOKE, 0.0, 0.0, 0.0, 0.0); break;
    case 1004: playLevelEventParticle(PARTICLE_FIREWORKS_SPARK, 0.0, 0.0, 0.0, 0.0); break;
    case 1005: playLevelEventParticle(PARTICLE_SMOKE, 0.0, random != NULL ? random->nextFloat() : 0.0, 0.0, 1.0); break;
    case 1006: playLevelEventParticle(PARTICLE_SMOKE, 0.0, random != NULL ? random->nextFloat() : 0.0, 0.0, 1.0); break;
    case 1007: playLevelEventParticle(PARTICLE_SMOKE, 0.0, random != NULL ? random->nextFloat() : 0.0, 0.0, 1.0); break;
    case 1008: playLevelEventParticle(PARTICLE_SMOKE, 0.0, random != NULL ? random->nextFloat() : 0.0, 0.0, 1.0); break;
    case 1009:
        playLevelEventParticle(PARTICLE_SMOKE,
                               random != NULL ? random->nextFloat() : 0.0,
                               random != NULL ? random->nextFloat() : 0.0,
                               random != NULL ? random->nextFloat() : 0.0, 1.0);
        break;
    case 1010:
    {
        if (data == 0)
        {
            level->playStreamingMusic(pos, NULL, false);
        }
        else
        {
            RecordItem* record = dynamic_cast<RecordItem*>(Item::byId(data));
            level->playStreamingMusic(pos, record != NULL ? record->getSound() : NULL, true);
        }
        if (player != NULL)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> local =
                boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(player);
            if (local != NULL) local->updateRichPresence();
        }
        break;
    }
    case 1011: playLevelEventParticle(PARTICLE_HUGE_EXPLOSION, 0.0, random != NULL ? random->nextFloat() : 0.0, 0.0, 1.0); break;
    case 1012: playLevelEventParticle(PARTICLE_LARGE_SMOKE, 0.0, random != NULL ? random->nextFloat() : 0.0, 0.0, 1.0); break;
    case 1013: playLevelEventParticle(PARTICLE_FLAME, 0.0, random != NULL ? random->nextFloat() : 0.0, 0.0, 1.0); break;
    case 1014: playLevelEventParticle(PARTICLE_SMOKE, 0.0, random != NULL ? random->nextFloat() : 0.0, 0.0, 1.0); break;
    case 1015: playLevelEventParticle(PARTICLE_LARGE_EXPLODE, random != NULL ? random->nextFloat() : 0.0, random != NULL ? random->nextFloat() : 0.0, 1.0, 1.0); break;
    case 1016: playLevelEventParticle(PARTICLE_END_PORTAL, random != NULL ? random->nextFloat() : 0.0, random != NULL ? random->nextFloat() : 0.0, 1.0, 1.0); break;
    case 1017: break;
    case 1018: playLevelEventParticle(PARTICLE_BLOCK_DUST, random != NULL ? random->nextFloat() : 0.0, random != NULL ? random->nextFloat() : 0.0, 1.0, 1.0); break;
    case 1019: playBlockEventSound(SoundEvents::ANVIL_BREAK, 1.0f, 1.0f, 16.0f); break;
    case 1020: playBlockEventSound(SoundEvents::ANVIL_USE, 1.0f, 1.0f, 16.0f); break;
    case 1021: playBlockEventSound(SoundEvents::ANVIL_LAND, 1.0f, 1.0f, 16.0f); break;
    case 1022:
    case 1023:
        break;
    case 1024: playBlockEventSound(SoundEvents::PORTAL_TRAVEL, 1.0f, 1.0f, 16.0f); break;
    case 1025: playBlockEventSound(SoundEvents::CHORUS_FLOWER_GROW, 1.0f, 1.0f, 16.0f); break;
    case 1026: playBlockEventSound(SoundEvents::CHORUS_FLOWER_DEATH, 1.0f, 1.0f, 16.0f); break;
    case 1027: playBlockEventSound(SoundEvents::END_PORTAL_SPAWN, 1.0f, 1.0f, 16.0f); break;
    case 1028:
    {
        boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
        if (camera != NULL)
        {
            addParticle(level, ePARTICLE_ENDER, false, camera->mX, camera->mY, camera->mZ,
                        0.0, 0.0, 0.0, arrayWithLength<int>());
        }
        break;
    }
    case 1029: playBlockEventSound(SoundEvents::ENDERCHEST_OPEN, 1.0f, 1.0f, 16.0f); break;
    case 1030: playBlockEventSound(SoundEvents::ENDERCHEST_CLOSE, 1.0f, 1.0f, 16.0f); break;
    case 1031: playBlockEventSound(SoundEvents::IRONGOLEM_ATTACK, 1.0f, 1.0f, 16.0f); break;
    case 1032: playBlockEventSound(SoundEvents::UI_BUTTON_CLICK, 1.0f, 1.0f, 16.0f); break;
    case 1033: playBlockEventSound(SoundEvents::IRONGOLEM_HURT, 1.0f, 1.0f, 16.0f); break;
    case 1034: playBlockEventSound(SoundEvents::IRONGOLEM_DEATH, 1.0f, 1.0f, 16.0f); break;
    case 1035: playBlockEventSound(SoundEvents::IRONGOLEM_STEP, 1.0f, 1.0f, 16.0f); break;
    case 1036: playBlockEventSound(SoundEvents::ITEMFRAME_ADD_ITEM, 1.0f, 1.0f, 16.0f); break;
    case 1037: playBlockEventSound(SoundEvents::ITEMFRAME_REMOVE_ITEM, 1.0f, 1.0f, 16.0f); break;
    case 2000:
    {
        int xOffset = data % 3 - 1;
        int zOffset = (data / 3) % 3 - 1;
        for (int i = 0; i < 5; ++i)
        {
            double r0 = random != NULL ? random->nextDouble() : 0.5;
            double r1 = random != NULL ? random->nextDouble() : 0.5;
            double r2 = random != NULL ? random->nextDouble() : 0.5;
            double r3 = random != NULL ? random->nextDouble() : 0.0;
            double g0 = random != NULL ? random->nextGaussian() : 0.0;
            double g1 = random != NULL ? random->nextGaussian() : 0.0;
            double g2 = random != NULL ? random->nextGaussian() : 0.0;
            double px = pos.x + 0.5 + xOffset * 0.2 + (r0 - 0.5) * 0.2;
            double py = pos.y + 0.5 + (r1 - 0.5) * 0.2;
            double pz = pos.z + 0.5 + zOffset * 0.2 + (r2 - 0.5) * 0.2;
            double xa = g0 * 0.0078125;
            double ya = g1 * 0.0078125;
            double za = g2 * 0.0078125;
            addParticle(level, ePARTICLE_BLOCK_CRACK, false, px, py, pz, xa, ya, za,
                        arrayWithLength<int>());
            addParticle(level, ePARTICLE_BLOCK_CRACK, false, px, py, pz, -xa, ya, -za,
                        arrayWithLength<int>());
        }
        break;
    }
    case 2001:
    {
        Block* block = Block::byId(data & 0xFFF);
        if (block != NULL)
        {
            BlockPos blockPos(pos.x, pos.y, pos.z);
            const SoundType* soundType = block->getSoundType();
            if (soundType != NULL)
            {
                level->playLocalSound((double)pos.x, (double)pos.y, (double)pos.z,
                                      soundType->getBreakSound(), SoundSource::eSoundSource_Blocks,
                                      (soundType->getVolume() + 0.5f) * 1.0f,
                                      soundType->getPitch() * 0.8f, false, 16.0f);
            }
            const BlockState* state = block->getBlockState(data >> 12 & 0xFF);
            if (state != NULL && mMinecraft->mParticleEngine != NULL)
            {
                mMinecraft->mParticleEngine->destroy(blockPos, state);
            }
        }
        break;
    }
    case 2002:
        break;
    case 2003:
    {
        if (random == NULL)
        {
            break;
        }

        int itemData[2] = { Item::getId(Items::enderEye), 0 };
        arrayWithLength<int> params(itemData, 2);
        for (int i = 0; i < 2; ++i)
        {
            double angle = random->nextDouble() * 6.283185307179586;
            addParticle(level, ePARTICLE_ICON_CRACK, false,
                        pos.x + 0.5 + std::cos(angle), pos.y + 0.5,
                        pos.z + 0.5 + std::sin(angle), std::cos(angle), 0.0,
                        std::sin(angle), params);
        }
        break;
    }
    case 2004:
    {
        if (random == NULL)
        {
            break;
        }

        for (int i = 0; i < 10; ++i)
        {
            addParticle(level, ePARTICLE_SMOKE, false,
                        pos.x + (random->nextDouble() - 0.5) * 2.0,
                        pos.y + (random->nextDouble() - 0.5) * 2.0,
                        pos.z + (random->nextDouble() - 0.5) * 2.0,
                        0.0, 0.0, 0.0, arrayWithLength<int>());
            addParticle(level, ePARTICLE_FLAME, false,
                        pos.x + (random->nextDouble() - 0.5) * 2.0,
                        pos.y + (random->nextDouble() - 0.5) * 2.0,
                        pos.z + (random->nextDouble() - 0.5) * 2.0,
                        0.0, 0.0, 0.0, arrayWithLength<int>());
        }
        break;
    }
    case 2005:
    {
        DyeItem::addGrowthParticles(level, pos, data);
        break;
    }
    case 2006:
    {
        if (random != NULL)
        {
            for (int i = 0; i < 50; ++i)
            {
                double angle = random->nextDouble() * 6.283185307179586;
                double radius = random->nextDouble() * 5.0;
                addParticleInternal(level, ePARTICLE_SPELL, false, false,
                                    pos.x + std::cos(angle) * radius,
                                    pos.y + 1.0,
                                    pos.z + std::sin(angle) * radius,
                                    std::cos(angle), random->nextDouble() * 0.3,
                                    std::sin(angle), arrayWithLength<int>());
            }
        }
        break;
    }
    case 2007:
        break;
    case 3501:
        playLevelEventParticle(PARTICLE_EXPLODE, 0.0,
                               random != NULL ? random->nextGaussian() * 0.2 + 1.0 : 1.0,
                               0.0, 0.0);
        if (random != NULL) CauldronBlock::spawnExplosionParticles(level, pos.x, pos.y, pos.z, random, data, 10);
        break;
    case 3502:
        playLevelEventParticle(PARTICLE_SPLASH, 0.0,
                               random != NULL ? random->nextFloat() * 0.1f + 0.9f : 0.9f,
                               0.0, 0.0);
        if (random != NULL) CauldronBlock::spawnSplashParticles(level, pos.x, pos.y, pos.z, random, data, 2);
        break;
    case 3503:
        playLevelEventParticle(PARTICLE_BUBBLE, 0.0,
                               random != NULL ? random->nextFloat() * 0.1f + 0.9f : 0.9f,
                               0.0, 0.0);
        if (random != NULL) CauldronBlock::spawnBubbleParticles(level, pos.x, pos.y, pos.z, random, data, 2);
        break;
    case 3504:
    case 3505:
        playLevelEventParticle(type == 3504 ? PARTICLE_INSTANT_SPELL : PARTICLE_WINNING_INSTANT_SPELL,
                               0.0, random != NULL ? random->nextFloat() * 0.1f + 0.9f : 0.9f,
                               0.0, 0.0);
        if (random != NULL) CauldronBlock::spawnPotionParticles(level, pos.x, pos.y, pos.z, random, data, 2);
        break;
    case 3506:
        playLevelEventParticle(PARTICLE_BUBBLE_POP, 0.0,
                               random != NULL ? random->nextFloat() * 0.1f + 0.9f : 0.9f,
                               0.0, 0.0);
        if (random != NULL) CauldronBlock::spawnBubbleParticles(level, pos.x, pos.y, pos.z, random, data, 2);
        break;
    case 3507:
        playLevelEventParticle(PARTICLE_SPLASH, 0.0,
                               random != NULL ? random->nextFloat() * 0.1f + 0.9f : 0.9f,
                               0.0, 0.0);
        if (random != NULL) CauldronBlock::spawnSplashParticles(level, pos.x, pos.y, pos.z, random, data, 2);
        break;
    case 3508:
        playLevelEventParticle(PARTICLE_BUBBLE, 0.0,
                               random != NULL ? random->nextFloat() * 0.1f + 0.9f : 0.9f,
                               0.0, 0.0);
        if (random != NULL) CauldronBlock::spawnBubbleParticles(level, pos.x, pos.y, pos.z, random, data, 2);
        break;
    case 3509:
        playLevelEventParticle(PARTICLE_CURRENT_DOWN, 0.0,
                               random != NULL ? random->nextFloat() * 0.1f + 0.9f : 0.9f,
                               0.0, 0.0);
        if (random != NULL) CauldronBlock::spawnBubbleParticles(level, pos.x, pos.y, pos.z, random, data, 2);
        break;
    case 9002:
    {
        DragonEggBlock::generateTeleportParticles(level, pos.x, pos.y, pos.z, data);
        break;
    }
    default:
        break;
    }
}
