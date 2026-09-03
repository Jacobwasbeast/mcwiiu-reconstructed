#include "game/world/block/PackedIceBlock.hpp"

#include "game/util/Random.hpp"

PackedIceBlock::PackedIceBlock()
    : Block(static_cast<Material*>(0))
{
    DerivedInit();
    mFriction = 0.98f;
}

PackedIceBlock::~PackedIceBlock()
{
}

int PackedIceBlock::getResourceCount(Random* random)
{
    return 0;
}
