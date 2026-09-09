#include "game/item/SmeltItemFunction.hpp"

#include "game/core/LootTableContext.hpp"
#include "game/item/FurnaceRecipes.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Random.hpp"

SmeltItemFunction::SmeltItemFunction(std::vector<LootItemCondition*>* conditions)
    : LootItemFunction(conditions)
{
}

not_null_ptr<ItemInstance> SmeltItemFunction::apply(const not_null_ptr<ItemInstance>& itemInstance,
                                                    Random* random,
                                                    LootTableContext& context)
{
    if (!itemInstance->isEmpty())
    {
        ItemInstance* result = FurnaceRecipes::getInstance()->getResult(itemInstance);
        if (!result->isEmpty())
        {
            not_null_ptr<ItemInstance> smelted = result->copy();
            smelted->setCount(itemInstance->getCount());
            return smelted;
        }
    }
    return itemInstance;
}
