#ifndef GAME_ITEM_ADDITEMRULEDEFINITION_HPP
#define GAME_ITEM_ADDITEMRULEDEFINITION_HPP

#include <vector>

#include "game/item/not_null_ptr.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

class AddEnchantmentRuleDefinition;
class Container;
class ItemInstance;

class AddItemRuleDefinition : public GameRuleDefinition
{
public:
    AddItemRuleDefinition();
    virtual ~AddItemRuleDefinition();

    not_null_ptr<ItemInstance> getItem();
    static bool addItemToContainer(boost::shared_ptr<Container> container,
                                   not_null_ptr<ItemInstance> item,
                                   int slot,
                                   bool randomSlot);
    bool addItemToContainer(boost::shared_ptr<Container> container, int slot);

    virtual void getChildren(std::vector<GameRuleDefinition*>* children);
    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);

private:
    int mItemId;
    int mCount;
    int mData;
    int m4JData;
    int mSlot;
    std::vector<AddEnchantmentRuleDefinition*> mEnchantments;
};

#endif
