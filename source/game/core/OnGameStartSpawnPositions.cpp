#include "game/core/OnGameStartSpawnPositions.hpp"

#include "game/core/SpawnPositionSet.hpp"

OnGameStartSpawnPositions::~OnGameStartSpawnPositions()
{
}

ConsoleGameRules::EGameRuleType OnGameStartSpawnPositions::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(29);
}

GameRuleDefinition* OnGameStartSpawnPositions::addChild(ConsoleGameRules::EGameRuleType type)
{
    GameRuleDefinition* pChild = NULL;

    if (type == 31)
    {
        pChild = new SpawnPositionSet();
        mChildren.push_back(pChild);
    }

    return pChild;
}

SpawnPositionSet* OnGameStartSpawnPositions::getSpawnSet(unsigned char type)
{
    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        SpawnPositionSet* pSpawnSet = static_cast<SpawnPositionSet*>(*it);

        if (pSpawnSet->getType() == type)
        {
            return pSpawnSet;
        }
    }

    return NULL;
}
