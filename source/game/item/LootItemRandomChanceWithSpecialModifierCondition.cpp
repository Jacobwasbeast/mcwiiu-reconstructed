#include "game/item/LootItemRandomChanceWithSpecialModifierCondition.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/LootTableContext.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

LootItemRandomChanceWithSpecialModifierCondition::LootItemRandomChanceWithSpecialModifierCondition(float chanceValue)
    : chance(chanceValue)
{
}

bool LootItemRandomChanceWithSpecialModifierCondition::applies(Random* random, LootTableContext& context)
{
    boost::shared_ptr<Entity> entity = context.mKillerEntity;
    int specialModifier = entity->mSpecialMultiplier;
    double multiplier = static_cast<double>(context.level->getSpecialMultiplier(specialModifier));

    return static_cast<double>(random->nextFloat()) < static_cast<double>(chance) * multiplier;
}
