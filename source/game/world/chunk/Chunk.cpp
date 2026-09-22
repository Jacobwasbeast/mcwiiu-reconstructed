#include "game/world/chunk/Chunk.hpp"

#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/core/Culler.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"

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
    if (mNonStack && mMutex != NULL)
    {
        EnterCriticalSection(mMutex);
        LevelRenderer* renderer = chunkLevelRenderer();
        if (renderer != NULL)
        {
            unsigned char references = renderer->decGlobalChunkRefCount(mX, mZ, mDimension, mLevel);
            mNonStack = false;
            if (references == 0)
            {
                unsigned long long index = LevelRenderer::getGlobalIndexForChunk(mX, mZ, mDimension, mLevel);
                if ((long long)index >= 0)
                    renderer->setGlobalChunkFlags(mX, mZ, mDimension, mLevel, 0);
            }
        }
        LeaveCriticalSection(mMutex);
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
        mMinX = x + 8;
        mMinZ = z + 8;
        mMinDimension = dimension + 8;
        mMaxX = x + 8;
        mMaxZ = z + 8;
        mMaxDimension = dimension + 8;
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
        if (renderer != NULL && mMutex != NULL)
        {
            EnterCriticalSection(mMutex);
            if (renderer->incGlobalChunkRefCount(x, z, dimension, mLevel) == 1)
                renderer->setGlobalChunkFlag(x, z, dimension, mLevel, 2, 0);
            LeaveCriticalSection(mMutex);
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

unsigned long long Chunk::rebuild(unsigned int* count)
{
    unsigned int rebuilt = 0;
    int y;
    for (y = 0; y < 16; ++y)
    {
        int x;
        for (x = 0; x < 16; ++x)
        {
            int z;
            for (z = 0; z < 16; ++z)
            {
                ++rebuilt;
            }
        }
    }
    if (count != NULL)
        *count = rebuilt;
    return (unsigned long long)(unsigned long)this;
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
