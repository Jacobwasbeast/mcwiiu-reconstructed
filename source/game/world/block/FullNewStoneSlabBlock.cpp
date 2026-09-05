#include "game/world/block/FullNewStoneSlabBlock.hpp"

FullNewStoneSlabBlock::FullNewStoneSlabBlock()
    : NewStoneSlabBlock()
{
    NewStoneSlabBlock::DerivedInit();
}

FullNewStoneSlabBlock::~FullNewStoneSlabBlock()
{
}

bool FullNewStoneSlabBlock::isFullSize() const
{
    return true;
}
