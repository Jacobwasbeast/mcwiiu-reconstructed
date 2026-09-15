#include "game/minigame/SpiralShapeRuleDefinition.hpp"

#include "game/core/GameRules_DoubleAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/core/SpiralShape.hpp"

SpiralShapeRuleDefinition::SpiralShapeRuleDefinition()
{
    GameRules_IntAttribute* minRadius = addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(46), &mMinRadius);
    minRadius->setMin(1);
    minRadius->setMax(24);

    GameRules_IntAttribute* maxRadius = addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(47), &mMaxRadius);
    maxRadius->setMin(1);
    maxRadius->setMax(24);

    GameRules_DoubleAttribute* angle = addDoubleAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(17), &mAngle);
    angle->setMin(0.0);
    angle->setMax(360.0);
}

SpiralShapeRuleDefinition::~SpiralShapeRuleDefinition() = default;

ConsoleGameRules::EGameRuleType SpiralShapeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(53);
}

LayerShape* SpiralShapeRuleDefinition::Generate() const
{
    return new SpiralShape(mMinRadius, mMaxRadius, mAngle * 3.1459 / 360.0);
}
