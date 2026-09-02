#ifndef GAME_WORLD_BLOCK_FJNETHERRACKBLOCKPRED_HPP
#define GAME_WORLD_BLOCK_FJNETHERRACKBLOCKPRED_HPP

#include "game/world/block/Predicate.hpp"

class Biome;

class FjNetherrackBlockPred : public Predicate<const Biome::BlockPair&>
{
public:
    virtual bool apply(const Biome::BlockPair& blockPair) const;
    virtual ~FjNetherrackBlockPred();
};

#endif
