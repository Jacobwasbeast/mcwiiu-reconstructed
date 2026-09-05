#ifndef GAME_WORLD_GENERATION_TALLGRASSFEATURE_HPP
#define GAME_WORLD_GENERATION_TALLGRASSFEATURE_HPP

#include "game/world/generation/Feature.hpp"

template <class T> class EnumProperty;

class TallGrassBlock
{
public:
    class Variant;
    static EnumProperty<Variant*>* VARIANT;
};

class BlockState;

class TallgrassFeature : public Feature
{
public:
    TallgrassFeature(TallGrassBlock::Variant* variant);
    virtual ~TallgrassFeature();
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    const BlockState* mState;
};

#endif
