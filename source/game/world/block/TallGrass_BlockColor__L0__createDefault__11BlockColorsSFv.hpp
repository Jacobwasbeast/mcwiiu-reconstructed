#ifndef GAME_WORLD_BLOCK_TALLGRASS_BLOCKCOLOR_CREATEDEFAULT_HPP
#define GAME_WORLD_BLOCK_TALLGRASS_BLOCKCOLOR_CREATEDEFAULT_HPP

#include "game/world/block/BlockColors.hpp"

class TallGrass_BlockColor__L0__createDefault__11BlockColorsSFv : public BlockColor
{
public:
    virtual int getColor(const BlockState* state, LevelSource* levelSource,
                         const BlockPos* pos, int tintIndex);
};

#endif
