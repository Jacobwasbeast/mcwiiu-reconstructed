#include "game/world/block/BlockPredicate.hpp"

#include "game/world/block/BlockState.hpp"

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
