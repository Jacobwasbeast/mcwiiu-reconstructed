#include "game/minigame/NullThemeRuleDefinition.hpp"

#include "game/world/generation/VoidLayerTheme.hpp"

NullThemeRuleDefinition::~NullThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType NullThemeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(59);
}

LayerTheme* NullThemeRuleDefinition::Generate() const
{
    return new VoidLayerTheme();
}
