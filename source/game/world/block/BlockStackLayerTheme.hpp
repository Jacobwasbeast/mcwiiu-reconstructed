#ifndef GAME_WORLD_BLOCK_BLOCKSTACKLAYERTHEME_HPP
#define GAME_WORLD_BLOCK_BLOCKSTACKLAYERTHEME_HPP

#include <vector>

class BlockState;
class BlockTransaction;
class Random;

class BlockStackLayerTheme
{
public:
    std::vector<const BlockState*> mStates;

    BlockStackLayerTheme(const std::vector<const BlockState*>& states);
    virtual void Apply(Random&, BlockTransaction*, int&, int&) const;
};

#endif
