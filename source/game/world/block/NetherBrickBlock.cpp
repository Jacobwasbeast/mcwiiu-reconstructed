#include "game/world/block/NetherBrickBlock.hpp"

#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

NetherBrickBlock::NetherBrickBlock()
    : Block(Material::STONE)
{
    DerivedInit();
}

const MaterialColor* NetherBrickBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return MaterialColor::NETHER;
}

NetherBrickBlock::~NetherBrickBlock()
{
}
