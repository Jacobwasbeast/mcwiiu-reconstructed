#include "game/world/block/PurpurSlabBlockFull.hpp"

PurpurSlabBlockFull::PurpurSlabBlockFull()
    : PurpurSlabBlock()
{
    PurpurSlabBlock::DerivedInit();
}

PurpurSlabBlockFull::~PurpurSlabBlockFull()
{
}

bool PurpurSlabBlockFull::isFullSize() const
{
    return true;
}
