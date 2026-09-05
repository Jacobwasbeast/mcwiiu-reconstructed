#include "game/item/BowlFoodItem.hpp"

#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

BowlFoodItem::BowlFoodItem(int nutrition)
    : FoodItem(nutrition, false)
{
    setMaxStackSize(1);
}

BowlFoodItem::~BowlFoodItem()
{
}

not_null_ptr<ItemInstance> BowlFoodItem::finishUsingItem(not_null_ptr<ItemInstance> itemInstance,
                                                         Level* level,
                                                         boost::shared_ptr<LivingEntity> entity)
{
    FoodItem::finishUsingItem(itemInstance, level, entity);
    if (!Minecraft::InMiniGame(eMiniGameId_Battle, false))
    {
        return not_null_ptr<ItemInstance>(new ItemInstance(Items::bowl));
    }
    return itemInstance;
}
