#ifndef GAME_ITEM_ITEMINSTANCECLONERBUILDER_HPP
#define GAME_ITEM_ITEMINSTANCECLONERBUILDER_HPP

#include "game/item/ItemInstance.hpp"

class ItemInstanceClonerBuilder
{
public:
    virtual not_null_ptr<ItemInstance> build() const;

private:
    ItemInstance itemInstance;
};

#endif
