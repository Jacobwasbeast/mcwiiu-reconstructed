#include "game/minigame/WorldPositionRuleDefinition.hpp"

#include "game/core/GameRules_IntAttribute.hpp"

WorldPositionRuleDefinition::WorldPositionRuleDefinition()
    : mX(0),
      mY(0),
      mZ(0)
{
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(25), &mX);

    GameRules_IntAttribute* height = addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(28), &mY);
    height->setMin(0);

    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(31), &mZ);
}

WorldPositionRuleDefinition::~WorldPositionRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType WorldPositionRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(15);
}
