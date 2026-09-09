#include "game/world/block/BowItem_Pull_ItemPropertyFunction.hpp"

#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

float BowItem_Pull_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance,
                                               Level*,
                                               boost::shared_ptr<LivingEntity> entity)
{
    if (entity != NULL && entity->getUseItem()->getItem() == Items::bow)
    {
        const int useDuration = itemInstance->getUseDuration();
        const int remainingTicks = entity->getUseItemRemainingTicks();
        return (float)(useDuration - remainingTicks) / (float)BowItem::GetMaxBowDuration();
    }

    return 0.6f;
}
