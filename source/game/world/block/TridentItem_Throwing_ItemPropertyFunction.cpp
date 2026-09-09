#include "game/world/block/TridentItem_Throwing_ItemPropertyFunction.hpp"

#include "game/entity/mob/LivingEntity.hpp"

float TridentItem_Throwing_ItemPropertyFunction::call(not_null_ptr<ItemInstance> itemInstance,
                                                      Level*,
                                                      boost::shared_ptr<LivingEntity> entity)
{
    if (entity && entity->mUseItem && entity->getUseItem() == itemInstance)
    {
        return 1.0f;
    }

    return 0.0f;
}
