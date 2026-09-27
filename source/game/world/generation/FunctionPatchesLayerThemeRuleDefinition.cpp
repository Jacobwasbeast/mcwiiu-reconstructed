#include "game/world/generation/FunctionPatchesLayerThemeRuleDefinition.hpp"

#include <string>

#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"

template <typename T>
std::wstring _toString(T value);

template <typename T>
T _fromString(const std::wstring& value);

static const FunctionPatchesLayerTheme::BlockFunction sFunctions[] =
{
    FunctionPatchesLayerTheme::GRASS_PATCHES,
    FunctionPatchesLayerTheme::HELLFIRE_PATCHES,
    FunctionPatchesLayerTheme::SNOWLAYER_PATCHES,
    FunctionPatchesLayerTheme::TNT_TRAPS,
    FunctionPatchesLayerTheme::CARPET_TRAP
};

FunctionPatchesLayerThemeRuleDefinition::FunctionPatchesLayerThemeRuleDefinition()
    : mFunction(-1)
    , mNoiseScale(5.0)
{
}

FunctionPatchesLayerThemeRuleDefinition::~FunctionPatchesLayerThemeRuleDefinition()
{
}

ConsoleGameRules::EGameRuleType FunctionPatchesLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_FunctionPatchesTheme;
}

void FunctionPatchesLayerThemeRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface,
                                                               unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 2);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_func, _toString(mFunction));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_scale, _toString(mNoiseScale));
}

void FunctionPatchesLayerThemeRuleDefinition::addAttribute(const std::wstring& name,
                                                            const std::wstring& value)
{
    if (name.compare(L"func") == 0)
    {
        if (value.compare(L"grass") == 0)
        {
            mFunction = 0;
        }

        if (value.compare(L"hellfire") == 0)
        {
            mFunction = 1;
        }

        if (value.compare(L"snowlayer") == 0)
        {
            mFunction = 2;
        }

        if (value.compare(L"tnt") == 0)
        {
            mFunction = 3;
        }

        if (value.compare(L"carpet") == 0)
        {
            mFunction = 4;
        }
    }
    else if (name.compare(L"scale") == 0)
    {
        mNoiseScale = _fromString<double>(value);
    }
    else
    {
        GameRuleDefinition::addAttribute(name, value);
    }
}

LayerTheme* FunctionPatchesLayerThemeRuleDefinition::Generate() const
{
    return new FunctionPatchesLayerTheme(0, mNoiseScale, sFunctions[mFunction]);
}
