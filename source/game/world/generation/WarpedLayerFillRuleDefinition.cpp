#include "game/world/generation/WarpedLayerFillRuleDefinition.hpp"

#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/generation/WarpedLayerFill.hpp"

WarpedLayerFillRuleDefinition::WarpedLayerFillRuleDefinition()
    : mNoiseScale(10.0f)
    , mOffset(4)
    , mThickness(1)
{
    GameRules_FloatAttribute* noiseScaleAttribute = addFloatAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(65), &mNoiseScale);
    noiseScaleAttribute->setMin(1.0f);
    noiseScaleAttribute->setMax(254.0f);

    GameRules_IntAttribute* offsetAttribute = addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(70), &mOffset);
    offsetAttribute->setMin(0);
    offsetAttribute->setMax(9);

    GameRules_IntAttribute* thicknessAttribute = addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(71), &mThickness);
    thicknessAttribute->setMin(1);
    thicknessAttribute->setMax(9);
}

WarpedLayerFillRuleDefinition::~WarpedLayerFillRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType WarpedLayerFillRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(57);
}

LayerFill* WarpedLayerFillRuleDefinition::Generate() const
{
    return new WarpedLayerFill(mNoiseScale, mOffset, mThickness);
}
