#ifndef GAME_CORE_FIREWORKSDISPENSEBEHAVIOR_HPP
#define GAME_CORE_FIREWORKSDISPENSEBEHAVIOR_HPP

#include "game/item/DefaultDispenseItemBehavior.hpp"

class FireworksDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv
    : public DefaultDispenseItemBehavior
{
public:
    virtual ~FireworksDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv();
    virtual not_null_ptr<ItemInstance> execute(BlockSource*, not_null_ptr<ItemInstance>, eOUTCOME&);
    virtual void playSound(BlockSource*, eOUTCOME);
};

#endif
