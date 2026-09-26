#include "game/world/block/ItemPropertyFunction_Cooldown.hpp"
#include "game/core/EntityType.hpp"

#include "game/item/ItemCooldowns.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"

float ItemPropertyFunction_Cooldown::call(not_null_ptr<ItemInstance> itemInstance,
                                          Level*,
                                          boost::shared_ptr<LivingEntity> entity)
{
    if (entity && entity->GetType() == EntityType::PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        return player->getCooldowns()->getCooldownPercent(itemInstance->getItem(), 0.0f);
    }
    return 0.0f;
}
