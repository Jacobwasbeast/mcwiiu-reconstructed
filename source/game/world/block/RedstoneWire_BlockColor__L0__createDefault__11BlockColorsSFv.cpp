#include "game/world/block/RedstoneWire_BlockColor__L0__createDefault__11BlockColorsSFv.hpp"

#include "game/world/block/BlockState.hpp"
#include "game/world/block/RedStoneWireBlock.hpp"

int RedstoneWire_BlockColor__L0__createDefault__11BlockColorsSFv::getColor(
    const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
{
    return RedStoneWireBlock::getColorForData(state->getValue(RedStoneWireBlock::POWER));
}
