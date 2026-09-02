#include "game/minigame/CompoundGameRuleDefinition.hpp"

#include <string>

#include "game/core/GameRule.hpp"
#include "game/item/CollectItemRuleDefinition.hpp"
#include "game/item/DistributeItemsRuleDefinition.hpp"
#include "game/item/PlaceContainerRuleDefinition.hpp"
#include "game/item/PopulateContainerRuleDefinition.hpp"
#include "game/minigame/CompleteAllRuleDefinition.hpp"
#include "game/world/block/UseBlockRuleDefinition.hpp"
#include "game/world/generation/UpdatePlayerRuleDefinition.hpp"

template<typename T> std::wstring _toString(T value);

CompoundGameRuleDefinition::CompoundGameRuleDefinition()
{
    mCompletedChild = NULL;
}

CompoundGameRuleDefinition::~CompoundGameRuleDefinition()
{
    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        delete *it;
    }
}

void CompoundGameRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        children->push_back(*it);
    }
}

void CompoundGameRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children, ConsoleGameRules::EGameRuleType type)
{
    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        if ((*it)->getActionType() == type)
        {
            children->push_back(*it);
        }
    }
}

GameRuleDefinition* CompoundGameRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* pChild = NULL;

    switch (type)
    {
    case 6:
        pChild = new PlaceContainerRuleDefinition();
        break;

    case 14:
        pChild = new DistributeItemsRuleDefinition();
        break;

    case 25:
        pChild = new UseBlockRuleDefinition();
        break;

    case 26:
        pChild = new CollectItemRuleDefinition();
        break;

    case 27:
        pChild = new CompleteAllRuleDefinition();
        break;

    case 28:
        pChild = new UpdatePlayerRuleDefinition();
        break;

    case 32:
        pChild = new PopulateContainerRuleDefinition();
        break;

    default:
        return NULL;
    }

    if (pChild == NULL)
    {
        return NULL;
    }

    mChildren.push_back(pChild);

    return pChild;
}

void CompoundGameRuleDefinition::populateGameRule(GameRulesInstance::EGameRulesInstanceType type, GameRule* gameRule)
{
    int index = 0;

    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        GameRule* pChildRule = new GameRule(*it, gameRule->mConnection);

        (*it)->populateGameRule(type, pChildRule);

        GameRule::_ValueType value;
        value.pGameRule = pChildRule;
        value.bIsGameRule = true;

        gameRule->setParameter(L"rule" + _toString(index), value);

        ++index;
    }

    GameRuleDefinition::populateGameRule(type, gameRule);
}

bool CompoundGameRuleDefinition::onUseBlock(GameRule* gameRule, int blockId, int x, int y, int z)
{
    bool bTriggered = false;

    for (GameRule::ParameterMap::iterator it = gameRule->mParameters.begin(); it != gameRule->mParameters.end(); ++it)
    {
        if (it->second.bIsGameRule)
        {
            if (it->second.pGameRule->getGameRuleDefinition()->onUseBlock(it->second.pGameRule, blockId, x, y, z) && !bTriggered)
            {
                bTriggered = true;
                mCompletedChild = it->second.pGameRule->getGameRuleDefinition();
            }
        }
    }

    return bTriggered;
}

bool CompoundGameRuleDefinition::onCollectItem(GameRule* gameRule, not_null_ptr<ItemInstance> item)
{
    bool bTriggered = false;

    for (GameRule::ParameterMap::iterator it = gameRule->mParameters.begin(); it != gameRule->mParameters.end(); ++it)
    {
        if (it->second.bIsGameRule)
        {
            if (it->second.pGameRule->getGameRuleDefinition()->onCollectItem(it->second.pGameRule, item) && !bTriggered)
            {
                bTriggered = true;
                mCompletedChild = it->second.pGameRule->getGameRuleDefinition();
            }
        }
    }

    return bTriggered;
}

void CompoundGameRuleDefinition::postProcessPlayer(boost::shared_ptr<Player> player)
{
    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        (*it)->postProcessPlayer(player);
    }
}
