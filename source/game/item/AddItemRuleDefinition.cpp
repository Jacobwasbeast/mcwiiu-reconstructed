#include "game/item/AddItemRuleDefinition.hpp"

#include <boost/pointer_cast.hpp>

#include "game/item/AddEnchantmentRuleDefinition.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"

AddItemRuleDefinition::AddItemRuleDefinition()
    : GameRuleDefinition()
{
    mCount = 1;
    mItemId = 0;
    m4JData = 0;
    mData = 0;
    mSlot = -1;
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_itemId, &mItemId);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_quantity, &mCount);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_auxValue, &mData);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_dataTag, &m4JData);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_slot, &mSlot);
}

AddItemRuleDefinition::~AddItemRuleDefinition()
{
}

not_null_ptr<ItemInstance> AddItemRuleDefinition::getItem()
{
    Item* item = Item::byId(mItemId);
    if (item != NULL) {
        int count = mCount;
        int maxStackSize = item->getMaxStackSize();
        if (maxStackSize < count) {
            count = maxStackSize;
        }
        not_null_ptr<ItemInstance> itemInstance(new ItemInstance(item, count, mData));
        itemInstance->set4JData(m4JData);
        std::vector<AddEnchantmentRuleDefinition*>::iterator it;
        for (it = mEnchantments.begin(); it != mEnchantments.end(); ++it) {
            (*it)->enchantItem(itemInstance);
        }
        return itemInstance;
    }
    return ItemInstance::EMPTY;
}

bool AddItemRuleDefinition::addItemToContainer(boost::shared_ptr<Container> container,
                                               not_null_ptr<ItemInstance> item,
                                               int slot,
                                               bool randomSlot)
{
    Random random;
    if (slot >= 0 && static_cast<unsigned int>(slot) < container->getContainerSize()) {
        container->setItem(static_cast<unsigned int>(slot), item);
        return true;
    }

    boost::shared_ptr<Inventory> inventory = boost::dynamic_pointer_cast<Inventory>(container);
    if (inventory) {
        return inventory->add(item);
    }

    if (!randomSlot) {
        unsigned int index;
        for (index = 0; index < container->getContainerSize(); ++index) {
            not_null_ptr<ItemInstance> existing = container->getItem(index);
            if (existing->isEmpty()) {
                container->setItem(index, item);
                return true;
            }
        }
        return false;
    }

    if (container->isFull()) {
        return false;
    }
    unsigned int index;
    do {
        index = static_cast<unsigned int>(random.nextInt(static_cast<int>(container->getContainerSize())));
    } while (!container->getItem(index)->isEmpty());
    container->setItem(index, item);
    return true;
}

bool AddItemRuleDefinition::addItemToContainer(boost::shared_ptr<Container> container, int slot)
{
    not_null_ptr<ItemInstance> item = getItem();
    if (item->isEmpty()) {
        return false;
    }
    int targetSlot = slot;
    if (mSlot >= 0 && static_cast<unsigned int>(mSlot) < container->getContainerSize()) {
        targetSlot = mSlot;
    }
    return addItemToContainer(container, item, targetSlot, false);
}

void AddItemRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    std::vector<AddEnchantmentRuleDefinition*>::iterator it;
    for (it = mEnchantments.begin(); it != mEnchantments.end(); ++it) {
        children->push_back(*it);
    }
}

ConsoleGameRules::EGameRuleType AddItemRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_AddItem;
}

GameRuleDefinition* AddItemRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    AddEnchantmentRuleDefinition* child = NULL;
    if (type == ConsoleGameRules::eGameRuleType_AddEnchantment) {
        child = new AddEnchantmentRuleDefinition();
        mEnchantments.push_back(child);
    }
    return child;
}
