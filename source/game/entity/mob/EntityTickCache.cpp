#include "game/entity/mob/EntityTickCache.hpp"

#include "game/client/render/Renderer.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

EntityTickCache::EntityTickCache()
{
    mMinX = 1;
    mMinY = 1;
    mMinZ = 1;
    mMaxX = -1;
    mMaxY = -1;
    mMaxZ = -1;
    mAccessedMinX = 0;
    mAccessedMinY = 0;
    mAccessedMinZ = 0;
    mAccessedMaxX = 0;
    mAccessedMaxY = 0;
    mAccessedMaxZ = 0;
    mLastCenterX = 0;
    mLastCenterY = 0;
    mLastCenterZ = 0;
    mElements = NULL;
    mBlockStates = NULL;
    mLastFrameUpdated = 0;
    mCacheChanged = true;
}

EntityTickCache::~EntityTickCache()
{
    delete[] mElements;
    delete[] mBlockStates;
}

int EntityTickCache::getIndexInCache(int x, int y, int z) const
{
    int depth = mMaxZ - mMinZ + 1;

    return (x - mMinX) * (mMaxY - mMinY + 1) * depth + (y - mMinY) * depth + (z - mMinZ);
}

void EntityTickCache::blockChanged(const BlockPos& pos)
{
    int x = pos.x;
    int y = pos.y;
    int z = pos.z;

    if (x >= mMinX && x <= mMaxX && y >= mMinY && y <= mMaxY && z >= mMinZ && z <= mMaxZ)
    {
        int index = getIndexInCache(x, y, z);
        unsigned short element = mLevel->getElement(x, y, z);

        mBlockStates[index] = Block::getStateByElementId(element);
        mElements[index] = element;
    }
}

void EntityTickCache::updateExtents(int x, int y, int z)
{
    if (x < mAccessedMinX)
    {
        mAccessedMinX = x;
    }

    if (y < mAccessedMinY)
    {
        mAccessedMinY = y;
    }

    if (z < mAccessedMinZ)
    {
        mAccessedMinZ = z;
    }

    if (x > mAccessedMaxX)
    {
        mAccessedMaxX = x;
    }

    if (y > mAccessedMaxY)
    {
        mAccessedMaxY = y;
    }

    if (z > mAccessedMaxZ)
    {
        mAccessedMaxZ = z;
    }
}

unsigned short EntityTickCache::getElement(int x, int y, int z)
{
    updateExtents(x, y, z);

    if (x >= mMinX && x <= mMaxX && y >= mMinY && y <= mMaxY && z >= mMinZ && z <= mMaxZ)
    {
        int index = getIndexInCache(x, y, z);
        unsigned short element = mElements[index];

        if (element == 0xFFFF)
        {
            element = mLevel->getElement(x, y, z);
            mElements[index] = element;
        }

        return element;
    }

    return 0xFFFF;
}

const BlockState* EntityTickCache::getBlockState(int x, int y, int z)
{
    updateExtents(x, y, z);

    if (x >= mMinX && x <= mMaxX && y >= mMinY && y <= mMaxY && z >= mMinZ && z <= mMaxZ)
    {
        int index = getIndexInCache(x, y, z);
        const BlockState* state = mBlockStates[index];

        if (state == NULL)
        {
            state = Block::getStateByElementId(getElement(x, y, z));
            mBlockStates[index] = state;
        }

        return state;
    }

    return NULL;
}

void EntityTickCache::clearCache()
{
    mMinX = 0;
    mMinY = 0;
    mMinZ = 0;
    mMaxX = 0;
    mMaxY = 0;
    mMaxZ = 0;

    delete[] mBlockStates;
    mBlockStates = NULL;

    delete[] mElements;
    mElements = NULL;
}

void EntityTickCache::buildCache(Level* level, int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    mLevel = level;

    int centerX = (minX + maxX) / 2;
    int centerY = (minY + maxY) / 2;
    int centerZ = (minZ + maxZ) / 2;

    int newMinX = minX;

    if (mAccessedMinX < newMinX)
    {
        newMinX = mAccessedMinX;
    }

    if (centerX - newMinX > 3)
    {
        newMinX = centerX - 3;
    }

    int newMinY = minY;

    if (mAccessedMinY < newMinY)
    {
        newMinY = mAccessedMinY;
    }

    if (centerY - newMinY > 3)
    {
        newMinY = centerY - 3;
    }

    int newMinZ = minZ;

    if (mAccessedMinZ < newMinZ)
    {
        newMinZ = mAccessedMinZ;
    }

    if (centerZ - newMinZ > 3)
    {
        newMinZ = centerZ - 3;
    }

    int newMaxX = maxX;

    if (newMaxX < mAccessedMaxX)
    {
        newMaxX = mAccessedMaxX;
    }

    if (newMaxX - centerX > 3)
    {
        newMaxX = centerX + 3;
    }

    int newMaxY = maxY;

    if (newMaxY < mAccessedMaxY)
    {
        newMaxY = mAccessedMaxY;
    }

    if (newMaxY - centerY > 3)
    {
        newMaxY = centerY + 3;
    }

    int newMaxZ = maxZ;

    if (newMaxZ < mAccessedMaxZ)
    {
        newMaxZ = mAccessedMaxZ;
    }

    if (newMaxZ - centerZ > 3)
    {
        newMaxZ = centerZ + 3;
    }

    bool chunksChanged = false;

    for (int chunkX = mMinX >> 4; chunkX <= mMaxX >> 4; ++chunkX)
    {
        for (int chunkZ = mMinZ >> 4; chunkZ <= mMaxZ >> 4; ++chunkZ)
        {
            if (level->getChunk(chunkX, chunkZ)->getElementFrameLastUpdated() >= mLastFrameUpdated)
            {
                chunksChanged = true;
            }
        }
    }

    bool boundsChanged = true;

    if (mMinX == newMinX && mMinY == newMinY && mMinZ == newMinZ && mMaxX == newMaxX && mMaxY == newMaxY &&
        mMaxZ == newMaxZ)
    {
        boundsChanged = false;
    }

    mCacheChanged = chunksChanged || boundsChanged;

    if (!mCacheChanged)
    {
        return;
    }

    int size = (newMaxX - newMinX + 1) * (newMaxY - newMinY + 1) * (newMaxZ - newMinZ + 1);

    if (size > 1024)
    {
        clearCache();

        return;
    }

    unsigned short* elements = NULL;
    const BlockState** blockStates = NULL;

    if (size > 0)
    {
        elements = new unsigned short[size];
        blockStates = new const BlockState*[size];
    }

    int index = 0;

    for (int x = newMinX; x <= newMaxX; ++x)
    {
        for (int y = newMinY; y <= newMaxY; ++y)
        {
            for (int z = newMinZ; z <= newMaxZ; ++z)
            {
                if (chunksChanged)
                {
                    elements[index] = 0xFFFF;
                    blockStates[index] = NULL;
                }
                else
                {
                    elements[index] = getElement(x, y, z);
                    blockStates[index] = getBlockState(x, y, z);
                }

                ++index;
            }
        }
    }

    mMinX = newMinX;
    mMinY = newMinY;
    mMinZ = newMinZ;
    mMaxX = newMaxX;
    mMaxY = newMaxY;
    mMaxZ = newMaxZ;

    if (centerX != mLastCenterX || centerY != mLastCenterY || centerZ != mLastCenterZ)
    {
        mAccessedMinX = newMaxX;
        mAccessedMinY = newMaxY;
        mAccessedMinZ = newMaxZ;
        mAccessedMaxX = newMinX;
        mAccessedMaxY = newMinY;
        mAccessedMaxZ = newMinZ;
    }

    mLastCenterX = centerX;
    mLastCenterY = centerY;
    mLastCenterZ = centerZ;

    delete[] mElements;
    mElements = elements;

    delete[] mBlockStates;
    mBlockStates = blockStates;

    mLastFrameUpdated = gRenderer->GetFramesSinceBoot();
}

void EntityTickCache::buildCache(Level* level, AABB* box)
{
    int minX = (int)box->mMinX;

    if (box->mMinX < (double)minX)
    {
        --minX;
    }

    int minY = (int)box->mMinY;

    if (box->mMinY < (double)minY)
    {
        --minY;
    }

    int minZ = (int)box->mMinZ;

    if (box->mMinZ < (double)minZ)
    {
        --minZ;
    }

    float x1 = (float)box->mMaxX;
    int maxX = (int)x1;

    if ((float)maxX < x1)
    {
        ++maxX;
    }

    float y1 = (float)box->mMaxY;
    int maxY = (int)y1;

    if ((float)maxY < y1)
    {
        ++maxY;
    }

    float z1 = (float)box->mMaxZ;
    int maxZ = (int)z1;

    if ((float)maxZ < z1)
    {
        ++maxZ;
    }

    buildCache(level, minX, minY, minZ, maxX, maxY, maxZ);
}
