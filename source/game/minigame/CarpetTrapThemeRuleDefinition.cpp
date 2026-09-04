#include "game/minigame/CarpetTrapThemeRuleDefinition.hpp"

#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/generation/CarpetTrapLayerTheme.hpp"

CarpetTrapThemeRuleDefinition::CarpetTrapThemeRuleDefinition()
    : mCarpetColour(0)
{
    GameRules_IntAttribute* carpetColourAttribute =
        addIntAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(63), &mCarpetColour);
    carpetColourAttribute->setMin(0);
    carpetColourAttribute->setMax(15);

    GameRules_FloatAttribute* noiseScaleAttribute =
        addFloatAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(65), &mNoiseScale);
    noiseScaleAttribute->setMin(0.0f);
    noiseScaleAttribute->setMax(100.0f);
}

CarpetTrapThemeRuleDefinition::~CarpetTrapThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType CarpetTrapThemeRuleDefinition::getActionType()
{
    return static_cast<ConsoleGameRules::EGameRuleType>(68);
}

LayerTheme* CarpetTrapThemeRuleDefinition::Generate() const
{
    return new CarpetTrapLayerTheme(5, mNoiseScale, mCarpetColour);
}
