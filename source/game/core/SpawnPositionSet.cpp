#include "game/core/SpawnPositionSet.hpp"

SpawnPositionSet::SpawnPositionSet()
    : mType(0)
{
    addUnsignedCharAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(51), &mType);
}

SpawnPositionSet::~SpawnPositionSet() = default;

ConsoleGameRules::EGameRuleType SpawnPositionSet::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(31);
}

unsigned char SpawnPositionSet::getType()
{
    return mType;
}
