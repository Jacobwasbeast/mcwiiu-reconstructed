#include "game/world/block/FullWoodSlabBlock.hpp"

FullWoodSlabBlock::FullWoodSlabBlock()
    : WoodSlabBlock()
{
    WoodSlabBlock::DerivedInit();
}

FullWoodSlabBlock::~FullWoodSlabBlock()
{
}

bool FullWoodSlabBlock::isFullSize() const
{
    return true;
}
