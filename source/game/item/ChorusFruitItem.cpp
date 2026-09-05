#include "game/item/ChorusFruitItem.hpp"

#include "game/core/Mth.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

ChorusFruitItem::ChorusFruitItem(int nutrition, float saturationModifier)
    : FoodItem(nutrition, saturationModifier, false)
{
}

ChorusFruitItem::~ChorusFruitItem()
{
}

not_null_ptr<ItemInstance> ChorusFruitItem::finishUsingItem(not_null_ptr<ItemInstance> itemInstance,
                                                            Level* level,
                                                            boost::shared_ptr<LivingEntity> entity)
{
    not_null_ptr<ItemInstance> result = FoodItem::finishUsingItem(itemInstance, level, entity);

    if (!level->mIsClientSide)
    {
        Random* random = entity->getRandom();
        for (int attempt = 0; attempt < 16; ++attempt)
        {
            double x = entity->mX + (random->nextDouble() - 0.5) * 64.0;
            double y = Mth::clamp(entity->mY + (double)(random->nextInt(16) - 8),
                                  0.0,
                                  (double)level->getHeight() - 1.0);
            double z = entity->mZ + (random->nextDouble() - 0.5) * 64.0;

            if (entity->randomTeleport(x, y, z))
            {
                level->levelEvent(2003, BlockPos(entity.get()), 0);
                break;
            }
        }
    }

    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        player->getCooldowns()->addCooldown(this, 20);
    }

    return result;
}
