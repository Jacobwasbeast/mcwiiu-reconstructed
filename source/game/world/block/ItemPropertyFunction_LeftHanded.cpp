#include "game/world/block/ItemPropertyFunction_LeftHanded.hpp"

#include "game/core/HumanoidArm.hpp"
#include "game/entity/mob/LivingEntity.hpp"

float ItemPropertyFunction_LeftHanded::call(not_null_ptr<ItemInstance> itemInstance,
                                            Level* level,
                                            boost::shared_ptr<LivingEntity> entity)
{
    return entity && entity->getMainArm() != HumanoidArm::RIGHT ? 1.0f : 0.0f;
}
