#include "game/world/OnInitialiseWorld.hpp"

OnInitialiseWorld::~OnInitialiseWorld()
{
}

ConsoleGameRules::EGameRuleType OnInitialiseWorld::getActionType()
{
    return ConsoleGameRules::eGameRuleType_OnInitialiseWorld;
}
