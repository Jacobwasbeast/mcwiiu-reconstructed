#include "game/world/block/BookshelfBlock.hpp"

#include "game/item/Items.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

BookshelfBlock::BookshelfBlock()
    : Block(Material::WOOD)
{
    DerivedInit();
}

BookshelfBlock::~BookshelfBlock()
{
}

int BookshelfBlock::getResourceCount(Random*)
{
    return 3;
}

Item* BookshelfBlock::getResource(const BlockState*, Random*, int)
{
    return Items::book;
}

TextureAtlasSprite* BookshelfBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return Block::getTexture(direction, state);
}
