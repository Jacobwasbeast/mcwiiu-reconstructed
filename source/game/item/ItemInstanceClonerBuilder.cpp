#include "game/item/ItemInstanceClonerBuilder.hpp"

ItemInstance ItemInstanceClonerBuilder::build() const
{
    return itemInstance.copy();
}
