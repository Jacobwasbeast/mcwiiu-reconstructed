#include "game/world/block/MultiBlockItem.hpp"

MultiBlockItem::MultiBlockItem(Block* block, Block* blockForAuxValue,
                               ItemInstanceResolver nameFunction,
                               ItemInstanceResolver descriptionFunction)
    : BlockItem(block),
      mBlockForAuxValue(blockForAuxValue),
      mNameFunction(nameFunction),
      mDescriptionFunction(descriptionFunction)
{
    setMaxDamage(0);
    setStackedByData(true);
}

MultiBlockItem::~MultiBlockItem()
{
}

int MultiBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}

int MultiBlockItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    if (mNameFunction != NULL)
    {
        return mNameFunction(itemInstance);
    }
    return BlockItem::getDescriptionId(itemInstance);
}

int MultiBlockItem::getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    if (mDescriptionFunction != NULL)
    {
        return mDescriptionFunction(itemInstance);
    }
    return BlockItem::getUseDescriptionId(itemInstance);
}
