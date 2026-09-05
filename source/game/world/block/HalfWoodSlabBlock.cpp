#include "game/world/block/HalfWoodSlabBlock.hpp"

HalfWoodSlabBlock::HalfWoodSlabBlock()
    : WoodSlabBlock()
{
    WoodSlabBlock::DerivedInit();
}

HalfWoodSlabBlock::~HalfWoodSlabBlock()
{
}

bool HalfWoodSlabBlock::isFullSize() const
{
    return false;
}
