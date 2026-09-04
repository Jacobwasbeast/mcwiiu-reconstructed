#include "game/world/block/BlockStackLayerThemeRuleDefinition.hpp"

#include <vector>

#include "game/world/block/BlockStackLayerTheme.hpp"

BlockStackLayerThemeRuleDefinition::~BlockStackLayerThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType BlockStackLayerThemeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(63);
}

LayerTheme* BlockStackLayerThemeRuleDefinition::Generate() const
{
    std::vector<const BlockState*> states;
    fillStack(states);

    return new BlockStackLayerTheme(states);
}
