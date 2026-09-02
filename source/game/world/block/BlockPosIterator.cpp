#include "game/world/block/BlockPosIterator.hpp"

#include <algorithm>

#include "game/world/block/BlockPos.hpp"

BlockPosIterator::BlockPosIterator(const BlockPos& from, const BlockPos& to)
{
    this->currentPos = NULL;
    this->minPos = new BlockPos(std::min(from.x, to.x), std::min(from.y, to.y), std::min(from.z, to.z));
    this->maxPos = new BlockPos(std::max(from.x, to.x), std::max(from.y, to.y), std::max(from.z, to.z));
}

BlockPosIterator::~BlockPosIterator()
{
    delete this->currentPos;
    delete this->minPos;
    delete this->maxPos;
}

bool BlockPosIterator::hasNext()
{
    if (this->currentPos != NULL && this->currentPos->equals(this->maxPos))
    {
        return false;
    }

    return true;
}

BlockPos* BlockPosIterator::next()
{
    if (this->currentPos == NULL)
    {
        this->currentPos = new BlockPos(*this->minPos);
    }
    else if (this->currentPos->x < this->maxPos->x)
    {
        *this->currentPos = BlockPos(this->currentPos->x + 1, this->currentPos->y, this->currentPos->z);
    }
    else if (this->currentPos->y < this->maxPos->y)
    {
        *this->currentPos = BlockPos(this->minPos->x, this->currentPos->y + 1, this->currentPos->z);
    }
    else if (this->currentPos->z < this->maxPos->z)
    {
        *this->currentPos = BlockPos(this->minPos->x, this->minPos->y, this->currentPos->z + 1);
    }
    else
    {
        *this->currentPos = BlockPos(this->currentPos->x, this->currentPos->y, this->currentPos->z);
    }

    return this->currentPos;
}
