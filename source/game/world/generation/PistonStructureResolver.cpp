#include "game/world/generation/PistonStructureResolver.hpp"

#include "game/core/Direction.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/PistonBaseBlock.hpp"

PistonStructureResolver::PistonStructureResolver(Level* level, const BlockPos& pistonPos,
                                                 const Direction* direction, bool extending)
    : mLevel(level), mPistonPos(pistonPos), mMoveDirection(direction)
{
    if (extending)
    {
        mBlockPos = pistonPos.relative(direction);
    }
    else
    {
        mMoveDirection = direction->getOpposite();
        mBlockPos = pistonPos.relative(direction, 2);
    }
}

void PistonStructureResolver::reorderListAtCollision(int collisionIndex, int distance)
{
    std::vector<BlockPos> before;
    std::vector<BlockPos> collided;
    std::vector<BlockPos> after;
    std::vector<BlockPos>::iterator begin = mToPush.begin();
    std::vector<BlockPos>::iterator end = mToPush.end();

    before.insert(before.end(), begin, begin + distance);
    collided.insert(collided.end(), end - collisionIndex, end);
    after.insert(after.end(), begin + distance, end - collisionIndex);

    mToPush.clear();
    mToPush.insert(mToPush.end(), before.begin(), before.end());
    mToPush.insert(mToPush.end(), collided.begin(), collided.end());
    mToPush.insert(mToPush.end(), after.begin(), after.end());
}

bool PistonStructureResolver::addBranchingBlocks(const BlockPos& pos)
{
    std::vector<const Direction*>::const_iterator direction = Direction::VALUES.begin();
    while (direction != Direction::VALUES.end())
    {
        if ((*direction)->getAxis() != mMoveDirection->getAxis())
        {
            if (!addBlockLine(pos.relative(*direction), *direction))
            {
                return false;
            }
        }
        ++direction;
    }
    return true;
}

bool PistonStructureResolver::addBlockLine(const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = mLevel->getBlockState(pos);
    if (state->getPistonPushReaction() == PISTON_PUSH_IGNORE)
    {
        return true;
    }

    if (!PistonBaseBlock::isPushable(state, mLevel, pos, mMoveDirection, false, direction))
    {
        return true;
    }

    if (pos.equals(mPistonPos))
    {
        return true;
    }

    std::vector<BlockPos>::const_iterator existing = mToPush.begin();
    while (existing != mToPush.end())
    {
        if (*existing == pos)
        {
            return true;
        }
        ++existing;
    }

    int distance = 1;
    if (state->getBlock()->getPistonPushReaction(state) == PISTON_PUSH_NORMAL)
    {
        while (true)
        {
            BlockPos next = pos.relative(mMoveDirection->getOpposite(), distance);
            const BlockState* nextState = mLevel->getBlockState(next);
            if (nextState->getPistonPushReaction() == PISTON_PUSH_IGNORE)
            {
                break;
            }

            if (!PistonBaseBlock::isPushable(nextState, mLevel, next, mMoveDirection, false,
                                             mMoveDirection->getOpposite()) || next.equals(mPistonPos))
            {
                break;
            }

            ++distance;
            if (mToPush.size() + distance > 12)
            {
                return false;
            }

            if (nextState->getBlock()->getPistonPushReaction(nextState) != PISTON_PUSH_NORMAL)
            {
                break;
            }
        }
        --distance;
    }
    else
    {
        distance = 0;
    }

    int start = static_cast<int>(mToPush.size());
    while (distance >= 0)
    {
        mToPush.push_back(pos.relative(mMoveDirection->getOpposite(), distance));
        --distance;
    }

    int index = 1;
    while (true)
    {
        BlockPos next = pos.relative(mMoveDirection, index);
        std::vector<BlockPos>::const_iterator collision = mToPush.begin();
        while (collision != mToPush.end() && !(*collision == next))
        {
            ++collision;
        }
        if (collision != mToPush.end())
        {
            int collisionIndex = static_cast<int>(collision - mToPush.begin());
            reorderListAtCollision(start, collisionIndex);
            collisionIndex += start;
            if (collisionIndex < 0)
            {
                return true;
            }

            int i = 0;
            while (i < collisionIndex)
            {
                const BlockState* collisionState = mLevel->getBlockState(mToPush.at(i));
                if (collisionState->getPistonPushReaction() == PISTON_PUSH_NORMAL
                    && !addBranchingBlocks(mToPush.at(i)))
                {
                    return false;
                }
                ++i;
            }
            return false;
        }

        const BlockState* nextState = mLevel->getBlockState(next);
        if (nextState->getPistonPushReaction() == PISTON_PUSH_IGNORE)
        {
            return true;
        }
        if (!PistonBaseBlock::isPushable(nextState, mLevel, next, mMoveDirection, true, mMoveDirection))
        {
            return false;
        }
        if (next.equals(mPistonPos))
        {
            return false;
        }
        if (nextState->getPistonPushReaction() == PISTON_PUSH_DESTROY)
        {
            mToDestroy.push_back(next);
            return true;
        }
        if (mToPush.size() >= 12)
        {
            return false;
        }
        mToPush.push_back(next);
        ++index;
    }
}

bool PistonStructureResolver::resolve()
{
    mToPush.clear();
    mToDestroy.clear();

    const BlockState* state = mLevel->getBlockState(mBlockPos);
    if (!PistonBaseBlock::isPushable(state, mLevel, mBlockPos, mMoveDirection, false, mMoveDirection))
    {
        if (state->getPistonPushReaction() == PISTON_PUSH_DESTROY)
        {
            mToDestroy.push_back(mBlockPos);
            return true;
        }
    }
    else if (addBlockLine(mBlockPos, mMoveDirection))
    {
        std::size_t index = 0;
        while (index < mToPush.size())
        {
            const BlockState* nextState = mLevel->getBlockState(mToPush.at(index));
            if (nextState->getPistonPushReaction() == PISTON_PUSH_NORMAL
                && !addBranchingBlocks(mToPush.at(index)))
            {
                return false;
            }
            ++index;
        }
        return true;
    }
    return false;
}

std::vector<BlockPos>& PistonStructureResolver::getToPush()
{
    return mToPush;
}

std::vector<BlockPos>& PistonStructureResolver::getToDestroy()
{
    return mToDestroy;
}
