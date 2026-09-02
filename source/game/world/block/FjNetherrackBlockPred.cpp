#include "game/world/block/FjNetherrackBlockPred.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/biome/Biome.hpp"

bool FjNetherrackBlockPred::apply(const Biome::BlockPair& blockPair) const
{
    static const int netherrackId = Block::getId(Blocks::netherrack);

    return blockPair.blockId == netherrackId;
}

FjNetherrackBlockPred::~FjNetherrackBlockPred()
{
}
