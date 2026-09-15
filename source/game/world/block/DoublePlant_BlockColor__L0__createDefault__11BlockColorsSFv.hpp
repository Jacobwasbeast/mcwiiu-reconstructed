#ifndef GAME_WORLD_BLOCK_DOUBLEPLANT_BLOCKCOLOR_L0_CREATEDEFAULT_11BLOCKCOLORSSFV_HPP
#define GAME_WORLD_BLOCK_DOUBLEPLANT_BLOCKCOLOR_L0_CREATEDEFAULT_11BLOCKCOLORSSFV_HPP

#include "game/world/block/BlockColors.hpp"

class DoublePlant_BlockColor__L0__createDefault__11BlockColorsSFv : public BlockColor
{
public:
    virtual int getColor(const BlockState* state, LevelSource* levelSource,
                         const BlockPos* pos, int tintIndex);
};

#endif
