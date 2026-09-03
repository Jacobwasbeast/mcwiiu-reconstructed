#ifndef GAME_WORLD_GENERATION_SHAFTSLAYERTHEME_HPP
#define GAME_WORLD_GENERATION_SHAFTSLAYERTHEME_HPP

#include <vector>

class BlockState;
class BlockTransaction;
class Random;

class ShaftsLayerTheme
{
public:
    ShaftsLayerTheme(int heightOffset, const std::vector<const BlockState*>& states);
    bool hasShaft(int x, int z) const;
    void Apply(Random& random, BlockTransaction* transaction, int& width, int& depth) const;

private:
    int mHeightOffset;
    std::vector<const BlockState*> mStates;
};

#endif
