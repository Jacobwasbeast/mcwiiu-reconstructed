#include "game/item/EnchantRandomEquipmentFunction.hpp"

#include "game/core/LootTableContext.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"

EnchantRandomEquipmentFunction::~EnchantRandomEquipmentFunction()
{
}

not_null_ptr<ItemInstance> EnchantRandomEquipmentFunction::apply(const not_null_ptr<ItemInstance>& itemInstance,
                                                                 Random* random,
                                                                 LootTableContext& context)
{
    boost::shared_ptr<Entity> entity = context.mThisEntity.lock();
    double specialMultiplier = context.mLevel->getSpecialMultiplier(entity->mDimension);

    if (random->nextFloat() < static_cast<float>(chance * specialMultiplier))
    {
        int level = static_cast<int>(5.0 + specialMultiplier * static_cast<float>(random->nextInt(18)));
        EnchantmentHelper::enchantItem(random, itemInstance, level, true);
    }

    return itemInstance;
}
