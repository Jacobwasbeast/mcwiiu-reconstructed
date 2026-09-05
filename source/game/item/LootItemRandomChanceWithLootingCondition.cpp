#include "game/item/LootItemRandomChanceWithLootingCondition.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/LootTableContext.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/util/Random.hpp"

LootItemRandomChanceWithLootingCondition::LootItemRandomChanceWithLootingCondition(float chanceValue, float lootingMultiplierValue)
    : chance(chanceValue), lootingMultiplier(lootingMultiplierValue)
{
}

bool LootItemRandomChanceWithLootingCondition::applies(Random* random, LootTableContext& context)
{
    int lootingLevel = 0;
    boost::shared_ptr<Entity> entity = context.getEntity(LootTableContext::KILLER);

    if (entity && (entity->GetType() & ENTITY_TYPE_LIVING_ENTITY) == ENTITY_TYPE_LIVING_ENTITY)
    {
        lootingLevel = EnchantmentHelper::getMobLooting(boost::dynamic_pointer_cast<LivingEntity>(entity));
    }

    return random->nextFloat() < chance + static_cast<float>(lootingLevel) * lootingMultiplier;
}
