#include "game/world/chunk/ActiveChunkBoundsChecker.hpp"

#include "game/util/AABB.hpp"

#include <string.h>

ActiveChunkBoundsChecker::ActiveChunkBoundsChecker()
{
    mCacheBitCount = CHUNK_CACHE_BITS;
    mCacheEvaluated = new unsigned char[CHUNK_CACHE_BYTES];
    mCacheResult = new unsigned char[mCacheBitCount / 8];
    memset(mCacheEvaluated, 0, mCacheBitCount / 8);
    memset(mCacheResult, 0, mCacheBitCount / 8);
    mCacheActive = false;
}

ActiveChunkBoundsChecker::~ActiveChunkBoundsChecker()
{
    delete[] mCacheEvaluated;
    delete[] mCacheResult;
}

void ActiveChunkBoundsChecker::ActivateCache()
{
    mCacheActive = true;
}

void ActiveChunkBoundsChecker::DeactivateCache()
{
    mCacheActive = false;
}

void ActiveChunkBoundsChecker::ClearCache()
{
    memset(mCacheEvaluated, 0, mCacheBitCount / 8);
    memset(mCacheResult, 0, mCacheBitCount / 8);
    DeactivateCache();
}

bool ActiveChunkBoundsChecker::IsChunkWithinActiveArea(int chunkX, int chunkZ, const std::vector<AABB*>& activeBounds)
{
    int cacheByte = 0;
    unsigned char cacheMask = 0;
    bool withinBounds = false;

    if (mCacheActive)
    {
        if (chunkX < -CHUNK_CACHE_RADIUS || chunkX >= CHUNK_CACHE_RADIUS ||
            chunkZ < -CHUNK_CACHE_RADIUS || chunkZ >= CHUNK_CACHE_RADIUS)
        {
            return false;
        }

        int cacheIndex = (chunkX + CHUNK_CACHE_RADIUS) * CHUNK_CACHE_DIAMETER + (chunkZ + CHUNK_CACHE_RADIUS);
        if (cacheIndex < 0 || cacheIndex >= mCacheBitCount)
        {
            return false;
        }

        cacheByte = cacheIndex / 8;
        cacheMask = (unsigned char)(0x80 >> (cacheIndex % 8));

        if ((mCacheEvaluated[cacheByte] & cacheMask) != 0)
        {
            return (mCacheResult[cacheByte] & cacheMask) != 0;
        }
    }

    if (activeBounds.size() > 0)
    {
        for (std::vector<AABB*>::const_iterator it = activeBounds.begin(); it != activeBounds.end(); ++it)
        {
            const AABB* bounds = *it;
            withinBounds = bounds->mMinX <= chunkX && chunkX <= bounds->mMaxX &&
                           bounds->mMinZ <= chunkZ && chunkZ <= bounds->mMaxZ;
            if (withinBounds)
            {
                break;
            }
        }
    }

    if (mCacheActive)
    {
        mCacheEvaluated[cacheByte] |= cacheMask;
        if (withinBounds)
        {
            mCacheResult[cacheByte] |= cacheMask;
        }
    }

    return withinBounds;
}
