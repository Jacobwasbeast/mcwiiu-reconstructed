#ifndef GAME_WORLD_BLOCK_HORIZONTALDIRECTIONALBLOCK_HPP
#define GAME_WORLD_BLOCK_HORIZONTALDIRECTIONALBLOCK_HPP

#include "game/world/block/Block.hpp"

class DirectionProperty;

class HorizontalDirectionalBlock : public Block
{
public:
    HorizontalDirectionalBlock(Material* material);
    HorizontalDirectionalBlock(Material* material, const MaterialColor* materialColor);
    virtual ~HorizontalDirectionalBlock();

    static void blockStaticCtor();

    static const DirectionProperty* FACING;
    static const DirectionProperty* FACING_PROPERTY;
};

#endif
