#include "game/world/generation/BasicPatchesLayerThemeRuleDefinition.hpp"

#include <vector>

#include "game/core/GameRules_DoubleAttribute.hpp"
#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/generation/BasicPatchesLayerTheme.hpp"

BasicPatchesLayerThemeRuleDefinition::BasicPatchesLayerThemeRuleDefinition()
    : mPatchThreshold(0.0f),
      mNoiseScale(5.0),
      mPatchDepth(0)
{
    GameRules_IntAttribute* depthAttribute = addIntAttribute(ConsoleGameRules::eGameRuleAttr_startHeight, &mPatchDepth);
    depthAttribute->setMax(9);

    GameRules_FloatAttribute* thresholdAttribute = addFloatAttribute(ConsoleGameRules::eGameRuleAttr_freq, &mPatchThreshold);
    thresholdAttribute->setMin(-1.1f);
    thresholdAttribute->setMax(1.1f);

    GameRules_DoubleAttribute* scaleAttribute = addDoubleAttribute(ConsoleGameRules::eGameRuleAttr_scale, &mNoiseScale);
    scaleAttribute->setMin(1.0);
    scaleAttribute->setMax(1000.0);
}

BasicPatchesLayerThemeRuleDefinition::~BasicPatchesLayerThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType BasicPatchesLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_BasicPatchesTheme;
}

LayerTheme* BasicPatchesLayerThemeRuleDefinition::Generate() const
{
    std::vector<const BlockState*> blocks;
    fillStack(blocks);

    return new BasicPatchesLayerTheme(5, mNoiseScale, mPatchDepth, blocks, mPatchThreshold);
}
