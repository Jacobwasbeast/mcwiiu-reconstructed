#include "game/world/block/RedFlowerBlock.hpp"

RedFlowerBlock::RedFlowerBlock()
{
    DerivedInit();
}

RedFlowerBlock::~RedFlowerBlock()
{
}

FlowerBlock::FlowerCategory* RedFlowerBlock::getFlowerCategory()
{
    return FlowerCategory::RED;
}
