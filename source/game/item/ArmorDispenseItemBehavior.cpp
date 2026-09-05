#include "game/item/ArmorDispenseItemBehavior.hpp"

#include "game/item/ArmorItem.hpp"
#include "game/item/ItemInstance.hpp"

ArmorDispenseItemBehavior::~ArmorDispenseItemBehavior()
{
}

not_null_ptr<ItemInstance> ArmorDispenseItemBehavior::execute(BlockSource* blockSource,
                                                               not_null_ptr<ItemInstance> itemInstance,
                                                               eOUTCOME& outcome)
{
    not_null_ptr<ItemInstance> result = ArmorItem::dispenseArmor(blockSource, itemInstance, outcome);

    if (!result->isEmpty())
    {
        return result;
    }

    return DefaultDispenseItemBehavior::execute(blockSource, itemInstance, outcome);
}
