#include "game/world/block/PrismarineSlabBlockFull.hpp"

PrismarineSlabBlockFull::PrismarineSlabBlockFull()
    : PrismarineSlabBlock()
{
    PrismarineSlabBlock::DerivedInit();
}

PrismarineSlabBlockFull::~PrismarineSlabBlockFull()
{
}

bool PrismarineSlabBlockFull::isFullSize() const
{
    return true;
}
