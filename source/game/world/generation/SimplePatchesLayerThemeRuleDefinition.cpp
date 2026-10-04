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
    GameRules_IntAttribute* blockIdAttribute = addIntAttribute(ConsoleGameRules::eGameRuleAttr_blockId, &mBlockId);
    blockIdAttribute->setMin(0);
    blockIdAttribute->setMax(255);

    addIntAttribute(ConsoleGameRules::eGameRuleAttr_blockAux, &mBlockAux);

    GameRules_FloatAttribute* noiseScaleAttribute = addFloatAttribute(ConsoleGameRules::eGameRuleAttr_scale, &mNoiseScale);
    noiseScaleAttribute->setMin(0.0f);
    noiseScaleAttribute->setMax(100.0f);

    GameRules_FloatAttribute* thresholdAttribute = addFloatAttribute(ConsoleGameRules::eGameRuleAttr_freq, &mPatchThreshold);
    thresholdAttribute->setMin(-1.0f);
    thresholdAttribute->setMax(1.0f);
}

SimplePatchesLayerThemeRuleDefinition::~SimplePatchesLayerThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType SimplePatchesLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_SimplePatchesTheme;
}

LayerTheme* SimplePatchesLayerThemeRuleDefinition::Generate() const
{
    Block* block = Block::byId(mBlockId);
    const BlockState* blockState = block->getBlockState(mBlockAux);
    return new SimplePatchesLayerTheme(5, static_cast<double>(mNoiseScale), blockState, mPatchThreshold);
}
