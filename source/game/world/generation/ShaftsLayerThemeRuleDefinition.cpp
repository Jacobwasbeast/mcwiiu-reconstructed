#include "game/world/generation/ShaftsLayerThemeRuleDefinition.hpp"

#include <vector>

#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/generation/ShaftsLayerTheme.hpp"

ShaftsLayerThemeRuleDefinition::ShaftsLayerThemeRuleDefinition()
    : mHeightOffset(0)
{
    GameRules_IntAttribute* heightOffsetAttribute = addIntAttribute(ConsoleGameRules::eGameRuleAttr_startHeight, &mHeightOffset);
    heightOffsetAttribute->setMin(-9);
    heightOffsetAttribute->setMax(9);
}

ShaftsLayerThemeRuleDefinition::~ShaftsLayerThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType ShaftsLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_ShaftsTheme;
}

LayerTheme* ShaftsLayerThemeRuleDefinition::Generate() const
{
    std::vector<const BlockState*> blocks;
    fillStack(blocks);

    return new ShaftsLayerTheme(mHeightOffset, blocks);
}
