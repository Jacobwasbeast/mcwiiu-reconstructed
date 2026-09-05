#include "game/item/LootingEnchantFunction.hpp"

#include <boost/shared_ptr.hpp>

#include "game/network/ClientboundChatPacket.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"

LootingEnchantFunction::LootingEnchantFunction(std::vector<LootItemCondition*>* conditions, RandomValueBounds count, int limit)
    : LootItemFunction(conditions)
{
    this->count = count;
    this->limit = limit;
}

not_null_ptr<ItemInstance> LootingEnchantFunction::apply(const not_null_ptr<ItemInstance>& itemInstance, Random* random, LootTableContext& context)
{
    const DamageSource* damageSource = context.mDamageSource;

    if (damageSource != NULL)
    {
        boost::shared_ptr<Entity> killer = damageSource->getEntity();

        if (killer && (killer->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
        {
            int lootingLevel = EnchantmentHelper::getMobLooting(boost::static_pointer_cast<LivingEntity>(killer));

            if (lootingLevel == 0)
            {
                return itemInstance;
            }

            float extraCount = (float)lootingLevel * Mth::nextFloat(random, count.min, count.max);

            itemInstance->grow(Math::round(extraCount));

            if (limit != 0 && limit < itemInstance->getCount())
            {
                itemInstance->setCount(limit);
            }
        }
    }

    return itemInstance;
}
