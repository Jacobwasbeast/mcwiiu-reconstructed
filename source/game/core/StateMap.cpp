#include "game/core/StateMap.hpp"

#include "game/world/block/BlockState.hpp"

StateMap::~StateMap() = default;

ModelResourceLocation* StateMap::getLocation(const BlockState* state)
{
    return NULL;
}
