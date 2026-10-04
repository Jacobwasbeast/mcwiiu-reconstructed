#include "game/world/block/BlockPosIterator.hpp"

#include <algorithm>

#include "game/world/block/BlockPos.hpp"

BlockPosIterator::BlockPosIterator(const BlockPos& from, const BlockPos& to)
{
    int minX = from.x;
    int minY = from.y;
    int minZ = from.z;
    if (to.x < minX)
    {
        minX = to.x;
    }
    if (to.y < minY)
    {
        minY = to.y;
    }
    if (to.z < minZ)
    {
        minZ = to.z;
    }
    this->currentPos = NULL;
    this->minPos = new BlockPos(minX, minY, minZ);
    int maxX = from.x;
    int maxY = to.y;
    int maxZ = to.z;
    if (maxX < to.x)
    {
        maxX = to.x;
    }
    if (to.y < from.y)
    {
        maxY = from.y;
    }
    if (to.z < from.z)
    {
        maxZ = from.z;
    }
    this->maxPos = new BlockPos(maxX, maxY, maxZ);
}

BlockPosIterator::~BlockPosIterator()
{
    delete this->currentPos;
    delete this->minPos;
    delete this->maxPos;
}

bool BlockPosIterator::hasNext()
{
    return this->currentPos == NULL || !this->currentPos->equals(this->maxPos);
}

BlockPos* BlockPosIterator::next()
{
    if (this->currentPos == NULL)
    {
        this->currentPos = new BlockPos(*this->minPos);
    }
    else
    {
        int x = this->currentPos->x;
        int y = this->currentPos->y;
        int z = this->currentPos->z;
        if (x < this->maxPos->x)
        {
            x = x + 1;
        }
        else if (y < this->maxPos->y)
        {
            x = this->minPos->x;
            y = y + 1;
        }
        else if (z < this->maxPos->z)
        {
            x = this->minPos->x;
            y = this->minPos->y;
            z = z + 1;
        }
        const BlockPos& moved = BlockPos(x, y, z);
        *this->currentPos = moved;
    }

    return this->currentPos;
}
