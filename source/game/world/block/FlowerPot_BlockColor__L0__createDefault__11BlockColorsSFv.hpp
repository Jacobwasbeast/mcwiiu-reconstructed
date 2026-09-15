#ifndef GAME_WORLD_BLOCK_FLOWERPOT_BLOCKCOLOR_L0_CREATEDEFAULT_HPP
#define GAME_WORLD_BLOCK_FLOWERPOT_BLOCKCOLOR_L0_CREATEDEFAULT_HPP

#include "game/world/block/BlockColors.hpp"

class FlowerPot_BlockColor__L0__createDefault__11BlockColorsSFv : public BlockColor
{
public:
    explicit FlowerPot_BlockColor__L0__createDefault__11BlockColorsSFv(BlockColors* blockColors);
    virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex);

private:
    BlockColors* mBlockColors;
};

#endif
