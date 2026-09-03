#include "game/world/block/NetherBrickBlock.hpp"

#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/MaterialColor.hpp"

NetherBrickBlock::NetherBrickBlock()
    : Block(0)
{
    Block::DerivedInit();
}

const MaterialColor* NetherBrickBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return MaterialColor::NETHER;
}

NetherBrickBlock::~NetherBrickBlock()
{
}
