#ifndef GAME_CORE_SPAWNEGGDISPENSEBEHAVIOR_HPP
#define GAME_CORE_SPAWNEGGDISPENSEBEHAVIOR_HPP

#include "game/item/DefaultDispenseItemBehavior.hpp"

class SpawnEggDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv
    : public DefaultDispenseItemBehavior
{
public:
    virtual ~SpawnEggDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv() = default;
    virtual not_null_ptr<ItemInstance> execute(BlockSource*, not_null_ptr<ItemInstance>, eOUTCOME&);
};

#endif
