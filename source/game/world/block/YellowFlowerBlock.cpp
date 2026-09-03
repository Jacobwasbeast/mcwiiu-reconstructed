#include "game/world/block/YellowFlowerBlock.hpp"

YellowFlowerBlock::YellowFlowerBlock()
{
    DerivedInit();
}

YellowFlowerBlock::~YellowFlowerBlock()
{
}

FlowerBlock::FlowerCategory* YellowFlowerBlock::getFlowerCategory()
{
    return FlowerCategory::YELLOW;
}
