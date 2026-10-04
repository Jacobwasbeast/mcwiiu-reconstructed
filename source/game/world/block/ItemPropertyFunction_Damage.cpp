#include "game/world/block/ItemPropertyFunction_Damage.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/core/Mth.hpp"

float ItemPropertyFunction_Damage::call(not_null_ptr<ItemInstance> itemInstance,
                                        Level*,
                                        boost::shared_ptr<LivingEntity>)
{
    return Mth::clamp((float)itemInstance->getDamageValue() / (float)itemInstance->getMaxDamage(), 0.0f, 1.0f);
}
