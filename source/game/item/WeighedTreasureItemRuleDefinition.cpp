#include "game/item/WeighedTreasureItemRuleDefinition.hpp"

#include "game/core/WeighedTreasure.hpp"
#include "game/item/AddEnchantmentRuleDefinition.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

WeighedTreasureItemRuleDefinition::WeighedTreasureItemRuleDefinition()
{
    mMinCount = 0;
    mMaxCount = 0;
    mWeight = 0;

    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(46), &mMinCount);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(47), &mMaxCount);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(48), &mWeight);
}

WeighedTreasureItemRuleDefinition::~WeighedTreasureItemRuleDefinition()
{
}

WeighedTreasure* WeighedTreasureItemRuleDefinition::getItem()
{
    WeighedTreasure* treasure = new WeighedTreasure(Item::byId(mItemId), mData, mMinCount, mMaxCount, mWeight);
    not_null_ptr<ItemInstance> item = treasure->getItem();
    item->set4JData(m4JData);

    for (std::vector<AddEnchantmentRuleDefinition*>::iterator it = mEnchantments.begin();
         it != mEnchantments.end(); ++it)
    {
        (*it)->enchantItem(item);
    }

    return treasure;
}

ConsoleGameRules::EGameRuleType WeighedTreasureItemRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(12);
}
