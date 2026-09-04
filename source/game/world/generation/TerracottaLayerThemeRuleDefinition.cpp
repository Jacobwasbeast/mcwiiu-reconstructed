#include "game/world/generation/TerracottaLayerThemeRuleDefinition.hpp"

TerracottaLayerThemePattern getPattern(const std::wstring& pattern);
TerracottaLayerThemeFunction getColourFunc(const std::wstring& function);

TerracottaLayerThemeRuleDefinition::TerracottaLayerThemeRuleDefinition()
{
    mPattern.assign(L"RANDOM:2");
    mColourFunction.assign(L"UNIFORM");
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(77), &mColourFunction);
    addStringAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(76), &mPattern);
}

TerracottaLayerThemeRuleDefinition::~TerracottaLayerThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType TerracottaLayerThemeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(65);
}

LayerTheme* TerracottaLayerThemeRuleDefinition::Generate() const
{
    TerracottaLayerThemePattern pattern = getPattern(mPattern);
    TerracottaLayerThemeFunction function = getColourFunc(mColourFunction);
    return new TerracottaLayerTheme(pattern, function);
}
