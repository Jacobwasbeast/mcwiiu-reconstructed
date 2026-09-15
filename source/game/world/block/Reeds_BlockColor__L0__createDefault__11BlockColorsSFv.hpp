#ifndef GAME_WORLD_BLOCK_REEDS_BLOCKCOLOR_L0_CREATEDEFAULT_11BLOCKCOLORSSFV_HPP
#define GAME_WORLD_BLOCK_REEDS_BLOCKCOLOR_L0_CREATEDEFAULT_11BLOCKCOLORSSFV_HPP

#include <stdint.h>

class BlockPos;
class BlockState;
class LevelSource;

class Reeds_BlockColor__L0__createDefault__11BlockColorsSFv
{
public:
    uint32_t getColor(const BlockState*, LevelSource*, const BlockPos*, int);
};

#endif
