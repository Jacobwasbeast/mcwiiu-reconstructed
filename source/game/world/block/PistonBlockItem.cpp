#include "game/world/block/PistonBlockItem.hpp"

PistonBlockItem::PistonBlockItem(Block* block)
    : BlockItem(block)
{
}

PistonBlockItem::~PistonBlockItem()
{
}

int PistonBlockItem::getLevelDataForAuxValue(int)
{
    return 7;
}
