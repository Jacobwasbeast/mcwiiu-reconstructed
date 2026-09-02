#ifndef GAME_ITEM_ITEMINSTANCECLONERBUILDER_HPP
#define GAME_ITEM_ITEMINSTANCECLONERBUILDER_HPP

#include "game/item/not_null_ptr.hpp"

class ItemInstance;

class ItemInstanceClonerBuilder
{
public:
    virtual ~ItemInstanceClonerBuilder();

    virtual not_null_ptr<ItemInstance> build() const;

private:
    ItemInstance itemInstance;
};

#endif
