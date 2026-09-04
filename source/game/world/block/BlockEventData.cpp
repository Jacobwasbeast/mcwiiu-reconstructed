#include "game/world/block/BlockEventData.hpp"
#include "game/world/block/Block.hpp"

BlockEventData::BlockEventData(const BlockPos& pos, Block* blockValue, int paramAValue, int paramBValue)
    : BlockPos(), block(blockValue), paramA(paramAValue), paramB(paramBValue)
{
    Vec3i::operator=(pos);
}

BlockPos BlockEventData::getPos()
{
    return BlockPos(*this);
}

int BlockEventData::getParamA()
{
    return paramA;
}

int BlockEventData::getParamB()
{
    return paramB;
}

Block* BlockEventData::getBlock()
{
    return block;
}

bool BlockEventData::equals(BlockEventData& other)
{
    return BlockPos::equals(other) && paramA == other.paramA && paramB == other.paramB && block == other.block;
}
