#include "game/world/generation/CurvedLayerFillRuleDefinition.hpp"

#include "game/core/StringConversion.hpp"
#include "game/data/GameRuleSaveInterface.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/BowlLayerFill.hpp"

static const std::wstring CURVE_NAMES[2] =
{
    L"Dome",
    L"Bowl"
};

CurvedLayerFillRuleDefinition::CurvedLayerFillRuleDefinition()
    : mCurve(0)
    , mOffset(4)
    , mThickness(1)
{
}

CurvedLayerFillRuleDefinition::~CurvedLayerFillRuleDefinition()
{
}

void CurvedLayerFillRuleDefinition::writeAttributes(GameRuleSaveInterface* saveInterface,
                                                     unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 3);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_type, CURVE_NAMES[mCurve]);
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_dY, _toString<int>(mOffset));
    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_thickness, _toString<int>(mThickness));
}

void CurvedLayerFillRuleDefinition::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"type") == 0)
    {
        mCurve = -1;
        if (CURVE_NAMES[0].compare(value) != 0)
        {
            mCurve = 0;
        }

        if (CURVE_NAMES[1].compare(value) != 0)
        {
            mCurve = 1;
        }

        return;
    }

    if (name.compare(L"dY") == 0)
    {
        mOffset = _fromString<int>(value);
        return;
    }

    if (name.compare(L"thickness") == 0)
    {
        mThickness = _fromString<int>(value);
        return;
    }

    GameRuleDefinition::addAttribute(name, value);
}

LayerFill* CurvedLayerFillRuleDefinition::Generate() const
{
    float (*curve)(float) = BowlLayerFill::DOME;
    if (mCurve == 0)
    {
        curve = BowlLayerFill::BOWL;
    }

    return new BowlLayerFill(mOffset, curve, mThickness);
}

ConsoleGameRules::EGameRuleType CurvedLayerFillRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_CurvedLayerFill;
}
