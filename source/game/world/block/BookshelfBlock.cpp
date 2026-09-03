#include "game/world/block/BookshelfBlock.hpp"

#include "game/item/Items.hpp"
#include "game/world/block/Blocks.hpp"

BookshelfBlock::BookshelfBlock()
    : Block(0)
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
