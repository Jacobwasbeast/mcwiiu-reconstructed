#include "game/core/OnGameStartSpawnPositions.hpp"

#include "game/core/SpawnPositionSet.hpp"

ConsoleGameRules::EGameRuleType OnGameStartSpawnPositions::getActionType()
{
    return ConsoleGameRules::eGameRuleType_OnGameStartSpawnPositions;
}

GameRuleDefinition* OnGameStartSpawnPositions::addChild(ConsoleGameRules::EGameRuleType type)
{
    SpawnPositionSet* pChild = NULL;

    if (type == ConsoleGameRules::eGameRuleType_SpawnPositionSet)
    {
        pChild = new SpawnPositionSet();
        mChildren.push_back(pChild);
    }

    return pChild;
}

SpawnPositionSet* OnGameStartSpawnPositions::getSpawnSet(unsigned char type)
{
    for (std::vector<GameRuleDefinition*>::iterator it = mChildren.begin(), end = mChildren.end(); it != end; ++it)
    {
        SpawnPositionSet* pSpawnSet = static_cast<SpawnPositionSet*>(*it);

        if (pSpawnSet->getType() == type)
        {
            return pSpawnSet;
        }
    }

    return NULL;
}
