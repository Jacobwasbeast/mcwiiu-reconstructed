#include "game/item/RandomItemSetRuleDefinition.hpp"

#include "game/core/WeighedTreasure.hpp"
#include "game/item/WeighedTreasureItemRuleDefinition.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelRuleset.hpp"

RandomItemSetRuleDefinition::RandomItemSetRuleDefinition()
{
    mDescriptionName = L"buckets_";
    mDataTag = 0;
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(49), &mDescriptionName);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(47), &mDataTag);
}

RandomItemSetRuleDefinition::~RandomItemSetRuleDefinition()
{
    for (std::vector<WeighedTreasureItemRuleDefinition*>::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
        delete *it;
    }
}

std::wstring RandomItemSetRuleDefinition::getId()
{
    return mDescriptionName;
}

void RandomItemSetRuleDefinition::populateContainer(boost::shared_ptr<Container> container, LevelRuleset* ruleset)
{
    if (mItems.empty())
    {
        if (ruleset != NULL)
        {
            RandomItemSetRuleDefinition* itemSet = ruleset->getRandomItemSet(mDescriptionName);
            if (itemSet != NULL)
            {
                itemSet->populateContainer(container, ruleset);
            }
        }
        return;
    }

    Random random;
    arrayWithLength<WeighedTreasure*> treasures(static_cast<unsigned int>(mItems.size()), true);
    for (unsigned int i = 0; i < treasures.length; ++i)
    {
        treasures.data[i] = mItems[i]->getItem();
    }

    WeighedTreasure::addChestItems(&random, treasures, container, mDataTag, false);
    for (unsigned int i = 0; i < treasures.length; ++i)
    {
        delete treasures.data[i];
    }
    delete[] treasures.data;
}

void RandomItemSetRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<WeighedTreasureItemRuleDefinition*>::iterator it = mItems.begin(); it != mItems.end(); ++it)
    {
        children->push_back(*it);
    }
}

ConsoleGameRules::EGameRuleType RandomItemSetRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(13);
}

GameRuleDefinition* RandomItemSetRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    WeighedTreasureItemRuleDefinition* item = NULL;
    if (type == static_cast<ConsoleGameRules::EGameRuleType>(12))
    {
        item = new WeighedTreasureItemRuleDefinition();
        mItems.push_back(item);
    }
    return item;
}
