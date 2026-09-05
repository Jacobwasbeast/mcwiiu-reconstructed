#include "game/item/PopulateContainerRuleDefinition.hpp"

#include <boost/pointer_cast.hpp>

#include "game/item/Container.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"

PopulateContainerRuleDefinition::PopulateContainerRuleDefinition()
    : GameRuleDefinition()
{
    mX = 0;
    mY = 0;
    mZ = 0;
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(25), &mX);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(28), &mY);
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(31), &mZ);
}

PopulateContainerRuleDefinition::~PopulateContainerRuleDefinition()
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mAddItems.begin(); it != mAddItems.end(); ++it)
    {
        delete *it;
    }
    for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin(); it != mRandomItemSets.end(); ++it)
    {
        delete *it;
    }
}

void PopulateContainerRuleDefinition::GetPos(int& x, int& y, int& z)
{
    x = mX;
    y = mY;
    z = mZ;
}

void PopulateContainerRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mAddItems.begin(); it != mAddItems.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* PopulateContainerRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* child = NULL;
    if (type == static_cast<ConsoleGameRules::EGameRuleType>(10))
    {
        AddItemRuleDefinition* addItem = new AddItemRuleDefinition();
        mAddItems.push_back(addItem);
        child = addItem;
    }
    else if (type == static_cast<ConsoleGameRules::EGameRuleType>(13))
    {
        RandomItemSetRuleDefinition* randomItemSet = new RandomItemSetRuleDefinition();
        mRandomItemSets.push_back(randomItemSet);
        child = randomItemSet;
    }
    return child;
}

bool PopulateContainerRuleDefinition::evaluateRule(Level* level, LevelRuleset* ruleset, const MiniGameDef& miniGameDef)
{
    (void)miniGameDef;
    BlockPos position(mX, mY, mZ);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(position);
    boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(blockEntity);
    if (container)
    {
        container->setChanged();
        for (std::vector<RandomItemSetRuleDefinition*>::iterator it = mRandomItemSets.begin(); it != mRandomItemSets.end(); ++it)
        {
            (*it)->populateContainer(container, ruleset);
        }
        unsigned int slot = 0;
        for (std::vector<AddItemRuleDefinition*>::iterator it = mAddItems.begin(); it != mAddItems.end(); ++it)
        {
            if (slot >= container->getContainerSize())
            {
                break;
            }
            (*it)->addItemToContainer(container, static_cast<int>(slot));
            ++slot;
        }
    }
    return true;
}

ConsoleGameRules::EGameRuleType PopulateContainerRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(32);
}
