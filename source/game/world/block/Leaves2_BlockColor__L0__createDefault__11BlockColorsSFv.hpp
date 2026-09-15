#ifndef GAME_WORLD_BLOCK_LEAVES2_BLOCKCOLOR_L0_CREATEDEFAULT_11BLOCKCOLORSSFV_HPP
#define GAME_WORLD_BLOCK_LEAVES2_BLOCKCOLOR_L0_CREATEDEFAULT_11BLOCKCOLORSSFV_HPP

#include "game/world/block/BlockColors.hpp"

class Leaves2_BlockColor__L0__createDefault__11BlockColorsSFv : public BlockColor
{
public:
    int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex) override;
};

#endif
