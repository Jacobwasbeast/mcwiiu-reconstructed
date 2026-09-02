#include "game/world/generation/LayerThemeRuleDefinition.hpp"

#include "game/client/render/TextureThemeRuleDefinition.hpp"
#include "game/minigame/CarpetTrapThemeRuleDefinition.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/minigame/NullThemeRuleDefinition.hpp"
#include "game/minigame/SchematicThemeRuleDefinition.hpp"
#include "game/world/block/BlockStackLayerThemeRuleDefinition.hpp"
#include "game/world/block/MushroomBlockThemeRuleDefinition.hpp"
#include "game/world/generation/BasicPatchesLayerThemeRuleDefinition.hpp"
#include "game/world/generation/FunctionPatchesLayerThemeRuleDefinition.hpp"
#include "game/world/generation/RainbowLayerThemeRuleDefinition.hpp"
#include "game/world/generation/ShaftsLayerThemeRuleDefinition.hpp"
#include "game/world/generation/SimplePatchesLayerThemeRuleDefinition.hpp"
#include "game/world/generation/TerracottaLayerThemeRuleDefinition.hpp"

static const std::wstring sPassNames[4] =
{
    L"Look",
    L"Decorations",
    L"Hazards",
    L"Lights"
};

LayerThemeRuleDefinition::LayerThemeRuleDefinition()
    : LayerAttributeRuleDefinition<LayerTheme*>(static_cast<ConsoleGameRules::EGameRuleType>(58), CreateFuncImpl)
    , mPassId(0xFFFFFFFF)
{
}

LayerThemeRuleDefinition::~LayerThemeRuleDefinition()
{
}

unsigned int LayerThemeRuleDefinition::GetPassId() const
{
    return mPassId;
}

void LayerThemeRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 1);

    saveInterface->saveAttribute(static_cast<ConsoleGameRules::EGameRuleAttr>(9), sPassNames[mPassId]);
}

void LayerThemeRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"pass") == 0)
    {
        if (sPassNames[0].compare(value) == 0)
        {
            mPassId = 0;
        }

        if (sPassNames[1].compare(value) == 0)
        {
            mPassId = 1;
        }

        if (sPassNames[2].compare(value) == 0)
        {
            mPassId = 2;
        }

        if (sPassNames[3].compare(value) == 0)
        {
            mPassId = 3;
        }
    }
    else
    {
        GameRuleDefinition::addAttribute(name, value);
    }
}

LayerAttributeChildRuleDefinition<LayerTheme*>* LayerThemeRuleDefinition::CreateFuncImpl(ConsoleGameRules::EGameRuleType type)
{
    switch (type)
    {
    case static_cast<ConsoleGameRules::EGameRuleType>(59):
        return new NullThemeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(61):
        return new ShaftsLayerThemeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(62):
        return new BasicPatchesLayerThemeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(63):
        return new BlockStackLayerThemeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(64):
        return new RainbowLayerThemeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(65):
        return new TerracottaLayerThemeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(66):
        return new FunctionPatchesLayerThemeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(67):
        return new SimplePatchesLayerThemeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(68):
        return new CarpetTrapThemeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(69):
        return new MushroomBlockThemeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(70):
        return new TextureThemeRuleDefinition();

    case static_cast<ConsoleGameRules::EGameRuleType>(71):
        return new SchematicThemeRuleDefinition();

    default:
        break;
    }

    return NULL;
}
