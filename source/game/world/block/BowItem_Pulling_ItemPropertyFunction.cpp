#include "game/world/block/BowItem_Pulling_ItemPropertyFunction.hpp"

#include "game/entity/mob/LivingEntity.hpp"

float BowItem_Pulling_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance,
                                                 Level* level,
                                                 boost::shared_ptr<LivingEntity> entity) const
{
    if (entity && entity->mUseItem && entity->getUseItem() == itemInstance)
    {
        return 1.0f;
    }

    return 0.6f;
}
