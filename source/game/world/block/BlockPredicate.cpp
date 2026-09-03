#include "game/world/block/BlockPredicate.hpp"

#include "game/world/block/BlockState.hpp"

BlockPredicate::BlockPredicate(Block* block)
    : mBlock(block)
{
}

BlockPredicate BlockPredicate::forBlock(Block* block)
{
    return BlockPredicate(block);
}

BlockPredicate::~BlockPredicate()
{
}

bool BlockPredicate::apply(const BlockState* state) const
{
    if (state != NULL && state->getBlock() == mBlock)
    {
        return true;
    }

    return false;
}
