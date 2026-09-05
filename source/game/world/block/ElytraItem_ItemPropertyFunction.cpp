#include "game/world/block/ElytraItem_ItemPropertyFunction.hpp"

#include "game/item/ElytraItem.hpp"

float ElytraItem_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance,
                                            Level*,
                                            boost::shared_ptr<LivingEntity>)
{
    return ElytraItem::isFlyEnabled(itemInstance) ? 0.25f : 1.0f;
}
