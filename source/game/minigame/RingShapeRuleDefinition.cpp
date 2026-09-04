#include "game/minigame/RingShapeRuleDefinition.hpp"

#include "game/core/GameRules_DoubleAttribute.hpp"
#include "game/core/RingShape.hpp"
#include "game/world/generation/LayerShapeRuleDefinition.hpp"

RingShapeRuleDefinition::RingShapeRuleDefinition()
    : mScale(0.75)
    , mChild(NULL)
{
    GameRules_DoubleAttribute* scaleAttribute = addDoubleAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(73), &mScale);
    scaleAttribute->setMin(0.0);
    scaleAttribute->setMax(2.0);
}

RingShapeRuleDefinition::~RingShapeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType RingShapeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(52);
}

GameRuleDefinition* RingShapeRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    mChild = LayerShapeRuleDefinition::CreateFuncImpl(type);
    return mChild;
}

LayerShape* RingShapeRuleDefinition::Generate() const
{
    return new RingShape(mChild->GetValue(), 22.857147f, static_cast<float>(mScale));
}
