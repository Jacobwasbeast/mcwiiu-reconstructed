#include "game/world/block/ItemPropertyFunction_Damage.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/core/Mth.hpp"

float ItemPropertyFunction_Damage::call(not_null_ptr<ItemInstance> itemInstance,
                                        Level*,
                                        boost::shared_ptr<LivingEntity>)
{
    const float damage = (float)itemInstance->getDamageValue();
    const float maxDamage = (float)itemInstance->getMaxDamage();
    return Mth::clamp(damage / maxDamage, 0.0f, 1.0f);
}
