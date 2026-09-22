#include "game/minigame/CombinationDefinitionRuleDefinition.hpp"

CombinationDefinitionRuleDefinition::~CombinationDefinitionRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType CombinationDefinitionRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(
        AnyCombinationOfRuleDefinition::getActionType() + 1);
}
