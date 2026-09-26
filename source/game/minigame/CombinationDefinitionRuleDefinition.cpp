#include "game/minigame/CombinationDefinitionRuleDefinition.hpp"

CombinationDefinitionRuleDefinition::~CombinationDefinitionRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType CombinationDefinitionRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_CombinationDefinition;
}
