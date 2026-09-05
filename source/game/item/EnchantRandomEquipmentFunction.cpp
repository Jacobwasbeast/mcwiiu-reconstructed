#include "game/item/EnchantRandomEquipmentFunction.hpp"

#include "game/core/LootTableContext.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

EnchantRandomEquipmentFunction::EnchantRandomEquipmentFunction(std::vector<LootItemCondition*>* conditions,
                                                               float chance)
    : LootItemFunction(conditions), chance(chance)
{
}

EnchantRandomEquipmentFunction::~EnchantRandomEquipmentFunction()
{
}

not_null_ptr<ItemInstance> EnchantRandomEquipmentFunction::apply(const not_null_ptr<ItemInstance>& itemInstance,
                                                                 Random* random,
                                                                 LootTableContext& context)
{
    float specialMultiplier = (float)context.mLevel->getSpecialMultiplier(context.mThisEntity->mSpecialMultiplier);

    if (random->nextFloat() < chance * specialMultiplier)
    {
        int level = (int)(5.0f + specialMultiplier * (float)random->nextInt(18));
        EnchantmentHelper::enchantItem(random, itemInstance, level, true);
    }

    return itemInstance;
}
