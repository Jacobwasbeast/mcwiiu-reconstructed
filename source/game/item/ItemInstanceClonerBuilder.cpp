#include "game/item/ItemInstanceClonerBuilder.hpp"

ItemInstanceClonerBuilder::ItemInstanceClonerBuilder(const ItemInstance& item)
    : mItem(item)
{
}

not_null_ptr<ItemInstance> ItemInstanceClonerBuilder::build() const
{
    return mItem.copy();
}
