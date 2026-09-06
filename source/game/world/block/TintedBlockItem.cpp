#include "game/world/block/TintedBlockItem.hpp"

#include "game/item/ItemInstance.hpp"

TintedBlockItem::TintedBlockItem(Block* block, bool stackedByData)
    : BlockItem(block)
{
    if (stackedByData)
    {
        setMaxDamage(0);
        setStackedByData(true);
    }
}

TintedBlockItem::~TintedBlockItem()
{
}

int TintedBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

TintedBlockItem* TintedBlockItem::setDescriptionPostfixes(arrayWithLength<int> postfixes)
{
    mDescriptionPostfixes.length = postfixes.length;
    mDescriptionPostfixes.data = new int[postfixes.length]();

    for (unsigned int i = 0; i < postfixes.length; ++i)
    {
        mDescriptionPostfixes.data[i] = postfixes.data[i];
    }

    return this;
}

int TintedBlockItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    int auxValue = itemInstance->getAuxValue();

    if (auxValue >= 0 && auxValue < (int)mDescriptionPostfixes.length)
    {
        return mDescriptionPostfixes.data[auxValue];
    }

    return BlockItem::getDescriptionId(itemInstance);
}

TextureAtlasSprite* TintedBlockItem::getIcon(int auxValue)
{
    return BlockItem::getIcon(auxValue);
}
