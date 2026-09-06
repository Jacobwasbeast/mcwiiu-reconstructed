#ifndef GAME_ITEM_WEIGHTEDTREASUREITEMRULEDEFINITION_HPP
#define GAME_ITEM_WEIGHTEDTREASUREITEMRULEDEFINITION_HPP

#include <vector>

#include "game/core/WeighedTreasure.hpp"

#define private protected
#include "game/item/AddItemRuleDefinition.hpp"
#undef private

class WeighedTreasureItemRuleDefinition : public AddItemRuleDefinition
{
public:
    WeighedTreasureItemRuleDefinition();
    virtual ~WeighedTreasureItemRuleDefinition();

    WeighedTreasure* getItem();
    virtual ConsoleGameRules::EGameRuleType getActionType();

private:
    int mMinCount;
    int mMaxCount;
    int mWeight;
};

#endif
