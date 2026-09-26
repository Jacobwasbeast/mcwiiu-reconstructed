#include "game/minigame/RingShapeRuleDefinition.hpp"

#include "game/core/GameRules_DoubleAttribute.hpp"
#include "game/core/RingShape.hpp"
#include "game/world/generation/LayerShapeRuleDefinition.hpp"

RingShapeRuleDefinition::RingShapeRuleDefinition()
    : mScale(0.75)
    , mChild(NULL)
{
    GameRules_DoubleAttribute* scaleAttribute = addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_holeSize, &mScale);
    scaleAttribute->setMin(0.0);
    scaleAttribute->setMax(2.0);
}

RingShapeRuleDefinition::~RingShapeRuleDefinition() = default;

ConsoleGameRules::EGameRuleType RingShapeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_RingShape;
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
