#include "game/world/block/BookshelfBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

BookshelfBlock::BookshelfBlock()
    : Block(Material::WOOD)
{
    DerivedInit();
}

int BookshelfBlock::getResourceCount(Random* random)
{
    return 3;
}

Item* BookshelfBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Items::book;
}

TextureAtlasSprite* BookshelfBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP || direction == Direction::DOWN)
    {
        return Blocks::planks->getTexture(direction);
    }

    return Block::getTexture(direction, state);
}

BookshelfBlock::~BookshelfBlock()
{
}
