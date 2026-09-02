#include "game/minigame/ActiveViewAreaRuleDefinition.hpp"

ActiveViewAreaRuleDefinition::ActiveViewAreaRuleDefinition()
{
}

ActiveViewAreaRuleDefinition::~ActiveViewAreaRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType ActiveViewAreaRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(76);
}
