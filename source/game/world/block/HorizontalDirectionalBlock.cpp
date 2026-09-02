#include "game/world/block/HorizontalDirectionalBlock.hpp"

#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

const DirectionProperty* HorizontalDirectionalBlock::FACING = NULL;
const DirectionProperty* HorizontalDirectionalBlock::FACING_PROPERTY = NULL;

HorizontalDirectionalBlock::HorizontalDirectionalBlock(Material* material)
    : Block(material)
{
}

HorizontalDirectionalBlock::HorizontalDirectionalBlock(Material* material, const MaterialColor* materialColor)
    : Block(material, materialColor)
{
}

HorizontalDirectionalBlock::~HorizontalDirectionalBlock()
{
}

void HorizontalDirectionalBlock::blockStaticCtor()
{
    FACING_PROPERTY = FACING;
}
