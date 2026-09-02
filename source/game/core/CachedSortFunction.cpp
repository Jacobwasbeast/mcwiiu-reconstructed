#include "game/core/CachedSortFunction.hpp"

#include "game/util/AABB.hpp"

CachedSortFunction::CachedSortFunction(arrayWithLength<double> values, const AABB* bounds)
    : mValues(values),
      mMin(bounds->mMinX, bounds->mMinY, bounds->mMinZ),
      mSize(BlockPos(bounds->mMaxX - bounds->mMinX,
                     bounds->mMaxY - bounds->mMinY,
                     bounds->mMaxZ - bounds->mMinZ)
                .offset(BlockPos(1, 1, 1)))
{
}

int CachedSortFunction::getIndex(const BlockPos& pos) const
{
    BlockPos offset = pos.subtract(mMin);

    return offset.y + offset.z * mSize.y + offset.x * mSize.y * mSize.z;
}
