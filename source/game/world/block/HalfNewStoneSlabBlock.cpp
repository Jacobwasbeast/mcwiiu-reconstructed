#include "game/world/block/HalfNewStoneSlabBlock.hpp"

HalfNewStoneSlabBlock::HalfNewStoneSlabBlock()
    : NewStoneSlabBlock()
{
    NewStoneSlabBlock::DerivedInit();
}

HalfNewStoneSlabBlock::~HalfNewStoneSlabBlock()
{
}

bool HalfNewStoneSlabBlock::isFullSize() const
{
    return false;
}
