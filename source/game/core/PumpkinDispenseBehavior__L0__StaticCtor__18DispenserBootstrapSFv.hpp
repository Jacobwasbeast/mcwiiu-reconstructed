#ifndef GAME_CORE_PUMPKINDISPENSEBEHAVIOR_L0_STATICCTOR_18DISPENSERBOOTSTRAPSFV_HPP
#define GAME_CORE_PUMPKINDISPENSEBEHAVIOR_L0_STATICCTOR_18DISPENSERBOOTSTRAPSFV_HPP

#include "game/core/DispenserBootstrap.hpp"

class PumpkinDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv
    : public DispenserBootstrap::OptionalDispenseItemBehavior
{
public:
    virtual ~PumpkinDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv();
    virtual not_null_ptr<ItemInstance> execute(BlockSource*, not_null_ptr<ItemInstance>, eOUTCOME&);
    virtual void playSound(BlockSource*, eOUTCOME);
};

#endif
