#include "game/world/OnInitialiseWorld.hpp"

OnInitialiseWorld::~OnInitialiseWorld()
{
}

ConsoleGameRules::EGameRuleType OnInitialiseWorld::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(30);
}
