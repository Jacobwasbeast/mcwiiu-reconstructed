#include "game/item/PotionDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv.hpp"

PotionDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::~PotionDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv()
{
}

not_null_ptr<ItemInstance>
PotionDispenseBehavior__L0__StaticCtor__18DispenserBootstrapSFv::dispense(
    BlockSource* source, not_null_ptr<ItemInstance> item)
{
    return mDefaultDispenseItemBehavior.dispense(source, item);
}
