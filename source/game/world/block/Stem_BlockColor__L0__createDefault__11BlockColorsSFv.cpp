#include "game/world/block/Stem_BlockColor__L0__createDefault__11BlockColorsSFv.hpp"

#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/StemBlock.hpp"

int Stem_BlockColor__L0__createDefault__11BlockColorsSFv::getColor(
    const BlockState*, LevelSource* levelSource, const BlockPos* pos, int)
{
    const BlockState* state = levelSource->getBlockState(*pos);
    const int age = state->getValue(StemBlock::AGE);
    return (age << 21) | (0xff00 - (age << 11)) | (age << 2);
}
