#ifndef GAME_ITEM_POTIONDISPENSEBEHAVIOR_L0_STATICCTOR_HPP
#define GAME_ITEM_POTIONDISPENSEBEHAVIOR_L0_STATICCTOR_HPP

#include "game/item/DispenseItemBehavior.hpp"
#include "game/item/DefaultDispenseItemBehavior.hpp"

class PotionDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv
    : public DispenseItemBehavior
{
public:
    virtual ~PotionDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv();
    virtual not_null_ptr<ItemInstance> dispense(BlockSource* source,
                                                  not_null_ptr<ItemInstance> item);

private:
    DefaultDispenseItemBehavior mDefaultDispenseItemBehavior;
};

#endif
