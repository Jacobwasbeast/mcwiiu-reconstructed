#ifndef GAME_CORE_SPAWNPOSITIONSET_HPP
#define GAME_CORE_SPAWNPOSITIONSET_HPP

#include "game/minigame/CompoundGameRuleDefinition.hpp"

class SpawnPositionSet : public CompoundGameRuleDefinition
{
public:
    SpawnPositionSet();

    virtual ~SpawnPositionSet();

    virtual ConsoleGameRules::EGameRuleType getActionType();

    unsigned char getType();

private:
    unsigned char mType;
};

#endif
