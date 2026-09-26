#include "game/minigame/PatchyShapeRuleDefinition.hpp"

#include "game/core/GameRules_DoubleAttribute.hpp"
#include "game/core/PatchyShape.hpp"

PatchyShapeRuleDefinition::PatchyShapeRuleDefinition()
    : mScale(5.0)
{
    GameRules_DoubleAttribute* scaleAttribute = addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_scale, &mScale);
    scaleAttribute->setMin(1.0);
    scaleAttribute->setMax(254.0);
}

PatchyShapeRuleDefinition::~PatchyShapeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType PatchyShapeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_PatchyShape;
}

LayerShape* PatchyShapeRuleDefinition::Generate() const
{
    return new PatchyShape(mScale);
}
