#include "game/world/generation/SimplePatchesLayerThemeRuleDefinition.hpp"

#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/generation/SimplePatchesLayerTheme.hpp"

SimplePatchesLayerThemeRuleDefinition::SimplePatchesLayerThemeRuleDefinition()
    : mBlockId(-1),
      mBlockAux(0),
      mNoiseScale(5.0f),
      mPatchThreshold(0.0f)
{
    GameRules_IntAttribute* blockIdAttribute = addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(12), &mBlockId);
    blockIdAttribute->setMin(0);
    blockIdAttribute->setMax(255);

    addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(63), &mBlockAux);

    GameRules_FloatAttribute* noiseScaleAttribute = addFloatAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(65), &mNoiseScale);
    noiseScaleAttribute->setMin(0.0f);
    noiseScaleAttribute->setMax(100.0f);

    GameRules_FloatAttribute* thresholdAttribute = addFloatAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(66), &mPatchThreshold);
    thresholdAttribute->setMin(-1.0f);
    thresholdAttribute->setMax(1.0f);
}

SimplePatchesLayerThemeRuleDefinition::~SimplePatchesLayerThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType SimplePatchesLayerThemeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(67);
}

LayerTheme* SimplePatchesLayerThemeRuleDefinition::Generate() const
{
    Block* block = Block::byId(mBlockId);
    const BlockState* blockState = block->getBlockState(mBlockAux);
    return new SimplePatchesLayerTheme(0, static_cast<double>(mNoiseScale), blockState, mPatchThreshold);
}
