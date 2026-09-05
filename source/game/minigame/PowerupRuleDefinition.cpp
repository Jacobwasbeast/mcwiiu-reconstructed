#include "game/minigame/PowerupRuleDefinition.hpp"

#include "game/item/AddItemRuleDefinition.hpp"

PowerupRuleDefinition::PowerupRuleDefinition()
{
}

PowerupRuleDefinition::~PowerupRuleDefinition()
{
}

void PowerupRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<AddItemRuleDefinition*>::iterator it = mPowerups.begin(); it != mPowerups.end(); ++it)
    {
        children->push_back(*it);
    }
}

GameRuleDefinition* PowerupRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    AddItemRuleDefinition* item = NULL;
    if (type == static_cast<ConsoleGameRules::EGameRuleType>(10))
    {
        item = new AddItemRuleDefinition();
        mPowerups.push_back(item);
    }
    return item;
}

void PowerupRuleDefinition::GiveRandomPowerup(boost::shared_ptr<Player> player)
{
    int powerupCount = static_cast<int>(mPowerups.size());
    boost::shared_ptr<Container> container = player->mInventory;
    mPowerups.at(static_cast<unsigned int>(mRandom.nextInt(powerupCount)))->addItemToContainer(container, -1);
}

ConsoleGameRules::EGameRuleType PowerupRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(73);
}
