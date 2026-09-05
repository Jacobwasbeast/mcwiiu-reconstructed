#include "game/world/block/PrismarineSlabBlockHalf.hpp"

PrismarineSlabBlockHalf::PrismarineSlabBlockHalf()
    : PrismarineSlabBlock()
{
    PrismarineSlabBlock::DerivedInit();
}

PrismarineSlabBlockHalf::~PrismarineSlabBlockHalf()
{
}

bool PrismarineSlabBlockHalf::isFullSize() const
{
    return false;
}
