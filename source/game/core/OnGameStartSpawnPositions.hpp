#ifndef GAME_CORE_ONGAMESTARTSPAWNPOSITIONS_HPP
#define GAME_CORE_ONGAMESTARTSPAWNPOSITIONS_HPP

#include "game/minigame/CompoundGameRuleDefinition.hpp"

class SpawnPositionSet;

class OnGameStartSpawnPositions : public CompoundGameRuleDefinition
{
public:
    virtual ~OnGameStartSpawnPositions();

    virtual ConsoleGameRules::EGameRuleType getActionType();

    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);

    SpawnPositionSet* getSpawnSet(unsigned char type);
};

#endif
