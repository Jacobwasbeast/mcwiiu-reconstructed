#include "game/minigame/StarShapeRuleDefinition.hpp"

#include "game/core/StarShape.hpp"

StarShapeRuleDefinition::StarShapeRuleDefinition()
    : mPoints(5)
{
    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(72), &mPoints);
}

StarShapeRuleDefinition::~StarShapeRuleDefinition() = default;

ConsoleGameRules::EGameRuleType StarShapeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(50);
}

LayerShape* StarShapeRuleDefinition::Generate() const
{
    return new StarShape(mPoints);
}
