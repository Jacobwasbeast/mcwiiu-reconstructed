#include "game/world/chunk/ActiveChunkAreaRuleDefinition.hpp"

ActiveChunkAreaRuleDefinition::ActiveChunkAreaRuleDefinition()
{
}

ActiveChunkAreaRuleDefinition::~ActiveChunkAreaRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType ActiveChunkAreaRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(18);
}
