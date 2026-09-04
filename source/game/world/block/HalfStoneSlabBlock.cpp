#include "game/world/block/HalfStoneSlabBlock.hpp"

HalfStoneSlabBlock::HalfStoneSlabBlock()
    : StoneSlabBlock()
{
    StoneSlabBlock::DerivedInit();
}

HalfStoneSlabBlock::~HalfStoneSlabBlock()
{
}

bool HalfStoneSlabBlock::isFullSize() const
{
    return false;
}
