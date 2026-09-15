#ifndef GAME_CORE_DYEDISPENSEBEHAVIOR_L0_STATICCTOR_18DISPENSERBOOTSTRAPSFV_HPP
#define GAME_CORE_DYEDISPENSEBEHAVIOR_L0_STATICCTOR_18DISPENSERBOOTSTRAPSFV_HPP

#include "game/core/DispenserBootstrap.hpp"

class DyeDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv
    : public DispenserBootstrap::OptionalDispenseItemBehavior
{
public:
    virtual ~DyeDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv();
    virtual not_null_ptr<ItemInstance> execute(BlockSource*, not_null_ptr<ItemInstance>, eOUTCOME&);
};

#endif
