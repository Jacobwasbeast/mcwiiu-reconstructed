#include "game/item/BookItem.hpp"
#include "game/item/ItemInstance.hpp"

BookItem::BookItem()
    : Item()
{
}

BookItem::~BookItem()
{
}

bool BookItem::isEnchantable(not_null_ptr<ItemInstance> itemInstance)
{
    return itemInstance->getCount() == 1;
}

int BookItem::getEnchantmentValue()
{
    return 1;
}
