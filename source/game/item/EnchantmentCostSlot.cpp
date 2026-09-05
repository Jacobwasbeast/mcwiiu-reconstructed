#include "game/item/EnchantmentCostSlot.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

class DyeColor
{
public:
    static const DyeColor* BLUE;
    static const DyeColor* byItemData(int itemData);
};

EnchantmentCostSlot::EnchantmentCostSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

EnchantmentCostSlot::~EnchantmentCostSlot()
{
}

bool EnchantmentCostSlot::mayPlace(not_null_ptr<ItemInstance> itemInstance)
{
    if (itemInstance->getItem() == Items::dye)
    {
        return DyeColor::byItemData(itemInstance->getAuxValue()) == DyeColor::BLUE;
    }
    return false;
}
