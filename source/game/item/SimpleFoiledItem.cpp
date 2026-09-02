#include "game/item/SimpleFoiledItem.hpp"

#include "game/item/not_null_ptr.hpp"

SimpleFoiledItem::SimpleFoiledItem()
{
}

SimpleFoiledItem::~SimpleFoiledItem()
{
}

bool SimpleFoiledItem::isFoil(not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}
