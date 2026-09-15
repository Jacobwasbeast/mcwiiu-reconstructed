#ifndef GAME_CORE_EMPTYBUCKETDISPENSEBEHAVIOR_L0_STATICCTOR_18DISPENSERBOOTSTRAPSFV_HPP
#define GAME_CORE_EMPTYBUCKETDISPENSEBEHAVIOR_L0_STATICCTOR_18DISPENSERBOOTSTRAPSFV_HPP

#include "game/item/DefaultDispenseItemBehavior.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockSource;
class ItemInstance;

class EmptyBucketDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv
    : public DefaultDispenseItemBehavior
{
public:
    virtual ~EmptyBucketDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv();
    virtual not_null_ptr<ItemInstance> execute(BlockSource* source, not_null_ptr<ItemInstance> item,
                                               eOUTCOME& outcome);
};

#endif
