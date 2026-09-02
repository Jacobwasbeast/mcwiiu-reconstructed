#ifndef GAME_ITEM_SIMPLEFOILEDITEM_HPP
#define GAME_ITEM_SIMPLEFOILEDITEM_HPP

#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;

class SimpleFoiledItem : public Item
{
public:
    SimpleFoiledItem();

    virtual ~SimpleFoiledItem();
    virtual bool isFoil(not_null_ptr<ItemInstance> itemInstance);
};

#endif
