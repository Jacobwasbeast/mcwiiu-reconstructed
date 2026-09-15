#ifndef GAME_ENTITY_PROJECTILE_FIREBALLDISPENSEBEHAVIOR_HPP
#define GAME_ENTITY_PROJECTILE_FIREBALLDISPENSEBEHAVIOR_HPP

#include "game/item/DefaultDispenseItemBehavior.hpp"

class FireballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv
    : public DefaultDispenseItemBehavior
{
public:
    virtual ~FireballDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv();
    virtual not_null_ptr<ItemInstance> execute(BlockSource*, not_null_ptr<ItemInstance>, eOUTCOME&);
    virtual void playSound(BlockSource*, eOUTCOME);
};

#endif
