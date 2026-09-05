#include "game/core/SetAttributesFunction.hpp"

#include "game/item/not_null_ptr.hpp"

not_null_ptr<ItemInstance> SetAttributesFunction::apply(const not_null_ptr<ItemInstance>& itemInstance,
                                                        Random* random,
                                                        LootTableContext& context)
{
    return itemInstance;
}
