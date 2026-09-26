#include "game/minigame/CarpetTrapThemeRuleDefinition.hpp"

#include "game/core/GameRules_FloatAttribute.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/generation/CarpetTrapLayerTheme.hpp"

CarpetTrapThemeRuleDefinition::CarpetTrapThemeRuleDefinition()
    : mCarpetColour(0)
{
    GameRules_IntAttribute* carpetColourAttribute =
        addIntAttribute(ConsoleGameRules::eGameRuleAttr_blockAux, &mCarpetColour);
    carpetColourAttribute->setMin(0);
    carpetColourAttribute->setMax(15);

    GameRules_FloatAttribute* noiseScaleAttribute =
        addFloatAttribute(ConsoleGameRules::eGameRuleAttr_scale, &mNoiseScale);
    noiseScaleAttribute->setMin(0.0f);
    noiseScaleAttribute->setMax(100.0f);
}

CarpetTrapThemeRuleDefinition::~CarpetTrapThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType CarpetTrapThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_CarpetTrapTheme;
}

LayerTheme* CarpetTrapThemeRuleDefinition::Generate() const
{
    return new CarpetTrapLayerTheme(5, mNoiseScale, mCarpetColour);
}
