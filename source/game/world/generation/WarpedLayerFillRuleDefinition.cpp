#include "game/world/generation/WarpedLayerFillRuleDefinition.hpp"

#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/generation/WarpedLayerFill.hpp"

WarpedLayerFillRuleDefinition::WarpedLayerFillRuleDefinition()
    : mNoiseScale(10.0f)
    , mOffset(4)
    , mThickness(1)
{
    GameRules_FloatAttribute* noiseScaleAttribute = addFloatAttribute(ConsoleGameRules::eGameRuleAttr_scale, &mNoiseScale);
    noiseScaleAttribute->setMin(1.0f);
    noiseScaleAttribute->setMax(254.0f);

    GameRules_IntAttribute* offsetAttribute = addIntAttribute(ConsoleGameRules::eGameRuleAttr_dY, &mOffset);
    offsetAttribute->setMin(0);
    offsetAttribute->setMax(9);

    GameRules_IntAttribute* thicknessAttribute = addIntAttribute(ConsoleGameRules::eGameRuleAttr_thickness, &mThickness);
    thicknessAttribute->setMin(1);
    thicknessAttribute->setMax(9);
}

WarpedLayerFillRuleDefinition::~WarpedLayerFillRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType WarpedLayerFillRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_WarpedLayerFill;
}

LayerFill* WarpedLayerFillRuleDefinition::Generate() const
{
    return new WarpedLayerFill(mNoiseScale, mOffset, mThickness);
}
