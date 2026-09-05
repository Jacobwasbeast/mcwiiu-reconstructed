#include "game/world/block/PurpurSlabBlockHalf.hpp"

PurpurSlabBlockHalf::PurpurSlabBlockHalf()
    : PurpurSlabBlock()
{
    PurpurSlabBlock::DerivedInit();
}

PurpurSlabBlockHalf::~PurpurSlabBlockHalf()
{
}

bool PurpurSlabBlockHalf::isFullSize() const
{
    return false;
}
