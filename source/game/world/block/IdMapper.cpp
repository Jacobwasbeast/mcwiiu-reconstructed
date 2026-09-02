#include "game/world/block/IdMapper.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"

IdMapper<const BlockState*>::IdMapper()
    : mBlockStates()
{
}

int IdMapper<const BlockState*>::getId(const BlockState* state)
{
    return state->mElementId;
}

const BlockState* IdMapper<const BlockState*>::byId(int id)
{
    const BlockState* state = mBlockStates[id];
    if (state == NULL)
    {
        return Block::byId(id >> 4)->getBlockState(id & 0xF);
    }

    return state;
}

void IdMapper<const BlockState*>::addMapping(BlockState* state, int id)
{
    state->mElementId = id;
    mBlockStates[id] = state;
}
