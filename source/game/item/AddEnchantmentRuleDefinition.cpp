#include "game/item/AddEnchantmentRuleDefinition.hpp"

#include "game/core/GameRules_IntAttribute.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

AddEnchantmentRuleDefinition::AddEnchantmentRuleDefinition()
    : GameRuleDefinition()
{
    mEnchantmentId = 0;
    mLevel = 0;

    GameRules_IntAttribute* enchantmentAttribute =
        addIntAttribute(ConsoleGameRules::eGameRuleAttr_enchantmentId, &mEnchantmentId);
    enchantmentAttribute->setMin(0);
    enchantmentAttribute->setMax(0xff);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_enchantmentLevel, &mLevel)->setMin(0);
}

unsigned int AddEnchantmentRuleDefinition::enchantItem(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
    {
        return 0;
    }

    Enchantment* enchantment = Enchantment::byId(mEnchantmentId);
    if (item->getItem() == Items::enchantedBook)
    {
        EnchantmentInstance instance(enchantment, mLevel);
        EnchantedBookItem::addEnchantment(item, &instance);
        return 0;
    }

    if (item->getItem()->isEnchantable(item) && enchantment != NULL && enchantment->canEnchant(item))
    {
        item->enchant(enchantment, mLevel);
        return 1;
    }

    return 0;
}

ConsoleGameRules::EGameRuleType AddEnchantmentRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_AddEnchantment;
}
