#include "game/item/SimpleFoiledItem.hpp"

SimpleFoiledItem::SimpleFoiledItem()
    : Item()
{
}

SimpleFoiledItem::~SimpleFoiledItem()
{
}

bool SimpleFoiledItem::isFoil(not_null_ptr<ItemInstance>)
{
    return true;
}
